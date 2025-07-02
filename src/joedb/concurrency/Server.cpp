#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/protocol_version.h"
#include "joedb/journal/File_Hasher.h"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/co_spawn.hpp>

#include <cstdio>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 const std::map<char, const char *> Server::request_description
 ////////////////////////////////////////////////////////////////////////////
 {
  {'H', "get fast SHA-256 hash code"},
  {'I', "get full SHA-256 hash code"},
  {'r', "read a range of bytes"},
  {'D', "get checkpoint"},
  {'E', "lock"},
  {'F', "pull"},
  {'G', "lock_pull"},
  {'L', "lock"},
  {'M', "unlock"},
  {'N', "lock_push"},
  {'O', "push_unlock"}
 };

 ////////////////////////////////////////////////////////////////////////////
 std::ostream &Server::Session::write_id(std::ostream &out) const
 ////////////////////////////////////////////////////////////////////////////
 {
  out << server.get_endpoint_path() << '('
      << get_server().client.get_journal_checkpoint() << "): " << id << ": ";

  return out;
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Session::Session
 ////////////////////////////////////////////////////////////////////////////
 (
  Server &server,
  boost::asio::local::stream_protocol::socket &&socket
 ):
  joedb::asio::Server::Session(server, std::move(socket)),
  state(State::not_locking)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Session::~Session()
 ////////////////////////////////////////////////////////////////////////////
 {
  try
  {
   if (state == State::locking)
   {
    log("removing lock held by dying session.\n");
    get_server().unlock(*this);
   }
  }
  catch (...)
  {
   // What should we do? maybe post the exception?
   // This may not be a problem any more with coroutines.
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::async_read
 ////////////////////////////////////////////////////////////////////////////
 (
  std::shared_ptr<Session> session,
  size_t offset,
  size_t size,
  Transfer_Handler handler
 )
 {
  boost::asio::async_read
  (
   session->socket,
   boost::asio::buffer(session->buffer.data + offset, size),
   [this, handler, session](std::error_code e, size_t s)
   {
    (this->*handler)(session, e, s);
   }
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::lock_dequeue()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!locked)
  {
   if (lock_queue.empty())
   {
    if (client.is_shared() && client_lock)
    {
     client_lock->unlock();
     client_lock.reset();
    }
   }
   else
   {
    locked = true;
    const std::shared_ptr<Session> session = lock_queue.front();
    lock_queue.pop();
    session->log("locking");

    if (!client_lock)
    {
     if (!writable_journal_client)
     {
      session->log("error: locking pull-only server");
      session->buffer.data[0] = 'R';
     }
     else
      client_lock.emplace(*writable_journal_client); // ??? takes_time
    }

    if (session->state == Session::State::waiting_for_lock_to_pull)
     start_pulling(session);

    session->state = Session::State::locking;
    refresh_lock_timeout(session);
   }
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::lock
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::shared_ptr<Session> session,
  const Session::State state
 )
 {
  if (session->state != Session::State::locking)
  {
   session->state = state;
   lock_queue.emplace(session);
   lock_dequeue();
  }
  else
   session->log("error: locking an already locked session");
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::unlock(Session &session)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (session.state == Session::State::locking)
  {
   session.log("unlocking");
   session.state = Session::State::not_locking;
   locked = false;
   lock_timeout_timer.cancel();

   lock_dequeue();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::lock_timeout_handler
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::shared_ptr<Session> session,
  const std::error_code error
 )
 {
  if (!error)
  {
   session->log("timeout");

   if (session->push_writer)
   {
    session->push_writer.reset();
    session->push_status = 't';
   }

   unlock(*session);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::refresh_lock_timeout(const std::shared_ptr<Session> session)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (lock_timeout.count() > 0 && session->state == Session::State::locking)
  {
   lock_timeout_timer.expires_after(lock_timeout);
   lock_timeout_timer.async_wait
   (
    [this, session](std::error_code e){lock_timeout_handler(session, e);}
   );
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::push_transfer_handler
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::shared_ptr<Session> session,
  const std::error_code error,
  const size_t bytes_transferred
 )
 {
  if (!error)
  {
   if (session->push_writer)
    session->push_writer->write(session->buffer.data, bytes_transferred); // ??? takes_time

   session->push_remaining_size -= int64_t(bytes_transferred);
   session->progress_bar->print_remaining(session->push_remaining_size);

   push_transfer(session);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::push_transfer(const std::shared_ptr<Session> session)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (session->push_remaining_size > 0)
  {
   refresh_lock_timeout(session);

   async_read
   (
    session,
    0,
    size_t(std::min(session->push_remaining_size, int64_t(session->buffer.size))),
    &Server::push_transfer_handler
   );
  }
  else
  {
   if (session->push_writer)
   {
    if (client_lock)
    {
     client_lock->get_journal().soft_checkpoint_at
     (
      session->push_writer->get_position()
     );

     // ??? takes_time
     if (client.is_shared() && session->unlock_after_push)
     {
      client_lock->checkpoint_and_push_unlock();
      client_lock.reset();
     }
     else
      client_lock->checkpoint_and_push();
    }

    session->push_writer.reset();
   }

   session->progress_bar.reset();

   session->buffer.data[0] = session->push_status;
   session->log("returning " + std::to_string(session->push_status));
   write_buffer_and_next_command(session, 1);

   if (session->unlock_after_push)
    unlock(*session);

   for (auto &waiting_session: waiting_sessions)
   {
    if
    (
     waiting_session->state == Session::State::waiting_for_push_to_pull &&
     waiting_session->pull_checkpoint < client.get_journal_checkpoint()
    )
    {
     waiting_session->state = Session::State::not_locking;
     start_pulling(waiting_session);
    }
   }

   waiting_sessions.clear();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::push_handler
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::shared_ptr<Session> session,
  const std::error_code error,
  const size_t bytes_transferred
 )
 {
  if (!error)
  {
   session->buffer.index = 0;
   const int64_t from = session->buffer.read<int64_t>();
   const int64_t until = session->buffer.read<int64_t>();

   if (locked && session->state != Session::State::locking)
   {
    session->log("trying to push while someone else is locking");
   }
   else if (!locked)
   {
    session->log("taking the lock for push attempt.");
    lock(session, Session::State::waiting_for_lock_to_push);
   }

   const bool conflict =
   (
    session->state != Session::State::locking ||
    from != client.get_journal().get_checkpoint()
   );

   session->log("pushing, from = " + std::to_string(from) + ", until = " + std::to_string(until));

   session->progress_bar.emplace(until - from, nullptr);

   if (!writable_journal_client)
    session->push_status = 'R';
   else if (conflict)
    session->push_status = 'C';
   else
   {
    if (client_lock)
    {
     session->push_writer.emplace
     (
      client_lock->get_journal().get_async_tail_writer()
     );
    }
   }

   session->push_remaining_size = until - from;

   push_transfer(session);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::read_transfer_handler
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::shared_ptr<Session> session,
  Async_Reader reader,
  const std::error_code error,
  const size_t bytes_transferred,
  const size_t offset
 )
 {
  if (!error)
  {
   session->progress_bar->print_remaining(reader.get_remaining());

   if (offset + reader.get_remaining() > 0)
   {
    const size_t size = reader.read // ??? takes_time
    (
     session->buffer.data + offset,
     session->buffer.size - offset
    );

    if (reader.is_end_of_file())
     session->log("error: unexpected end of file");
    else
    {
     refresh_lock_timeout(session);

     boost::asio::async_write
     (
      session->socket,
      boost::asio::buffer(session->buffer.data, size + offset),
      [this, session, reader](std::error_code e, size_t s)
      {
       read_transfer_handler(session, reader, e, s, 0);
      }
     );
    }
   }
   else
   {
    session->pull_timer.reset();
    session->progress_bar.reset();
    read_command(session);
   }
  }
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::start_reading
 ///////////////////////////////////////////////////////////////////////////
 (
  std::shared_ptr<Session> session,
  Async_Reader reader
 )
 {
  session->log("reading from = " + std::to_string(reader.get_current()) + ", until = " + std::to_string(reader.get_end()));
  session->progress_bar.emplace(reader.get_remaining(), nullptr);

  session->buffer.index = 1;
  session->buffer.write<int64_t>(reader.get_end());

  read_transfer_handler
  (
   session,
   reader,
   std::error_code(),
   0,
   session->buffer.index
  );
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::start_pulling(std::shared_ptr<Session> session)
 ///////////////////////////////////////////////////////////////////////////
 {
  if
  (
   session->lock_before_pulling &&
   session->state != Session::State::waiting_for_lock_to_pull
  )
  {
   lock(session, Session::State::waiting_for_lock_to_pull);
   return;
  }

  if (!session->send_pull_data)
   session->pull_checkpoint = client.get_journal_checkpoint();

  start_reading
  (
   session,
   client.get_journal().get_async_tail_reader(session->pull_checkpoint)
  );
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::pull_handler
 ///////////////////////////////////////////////////////////////////////////
 (
  const std::shared_ptr<Session> session,
  const std::error_code error,
  const size_t bytes_transferred
 )
 {
  if (!error)
  {
   session->buffer.index = 1;
   const std::chrono::milliseconds wait{session->buffer.read<int64_t>()};
   session->pull_checkpoint = session->buffer.read<int64_t>();

   if (!client_lock)
    client.pull(); // ??? takes_time

   if
   (
    wait.count() > 0 &&
    session->pull_checkpoint == client.get_journal_checkpoint()
   )
   {
    session->log
    (
     "waiting at checkpoint = " + std::to_string(session->pull_checkpoint) +
     " for " + std::to_string(double(wait.count()) * 0.001) + 's'
    );

    session->state = Session::State::waiting_for_push_to_pull;
    session->pull_timer.emplace(io_context);
    session->pull_timer->expires_after(wait);
    session->pull_timer->async_wait
    (
     [this, session](std::error_code timer_error)
     {
      if (!timer_error)
      {
       if (session->state == Session::State::waiting_for_push_to_pull)
       {
        session->state = Session::State::not_locking;
        start_pulling(session);
       }
      }
     }
    );
   }
   else
    start_pulling(session);
  }
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::read_handler
 ///////////////////////////////////////////////////////////////////////////
 (
  std::shared_ptr<Session> session,
  std::error_code error,
  size_t bytes_transferred
 )
 {
  if (!error)
  {
   session->buffer.index = 1;
   int64_t from = session->buffer.read<int64_t>();
   int64_t until = session->buffer.read<int64_t>();

   if (until > client.get_journal_checkpoint())
    until = client.get_journal_checkpoint();
   if (from > until)
    from = until;

   start_reading
   (
    session,
    client.get_journal().get_async_reader(from, until)
   );
  }
 }

 ///////////////////////////////////////////////////////////////////////////
 boost::asio::awaitable<void> Server::Session::check_hash()
 ///////////////////////////////////////////////////////////////////////////
 {
  co_await boost::asio::async_read
  (
   socket,
   boost::asio::buffer(buffer.data + 1, 40),
   boost::asio::use_awaitable
  );

  buffer.index = 1;
  const auto checkpoint = buffer.read<int64_t>();
  const auto hash = buffer.read<SHA_256::Hash>();

  const Readonly_Journal &journal = get_server().client.get_journal();

  if
  (
   checkpoint > journal.get_checkpoint() ||
   hash != (buffer.data[0] == 'H' // ??? takes_time
   ? Journal_Hasher::get_fast_hash(journal, checkpoint)
   : Journal_Hasher::get_full_hash(journal, checkpoint))
  )
  {
   buffer.data[0] = 'h';
  }

  log
  (
   "hash for checkpoint = " + std::to_string(checkpoint) +
   ", result = " + std::to_string(buffer.data[0])
  );

  co_await boost::asio::async_write
  (
   socket,
   boost::asio::buffer(buffer.data, buffer.index),
   boost::asio::use_awaitable
  );
 }

 ///////////////////////////////////////////////////////////////////////////
 boost::asio::awaitable<void> Server::Session::handshake()
 ///////////////////////////////////////////////////////////////////////////
 {
  co_await boost::asio::async_read
  (
   socket,
   boost::asio::buffer(buffer.data, 13),
   boost::asio::use_awaitable
  );

  buffer.index = 0;

  if (buffer.read<std::array<char, 5>>() != Header::joedb)
   throw Exception("handshake does not start by joedb");

  const int64_t client_version = buffer.read<int64_t>();
  log("client_version = " + std::to_string(client_version));

  const int64_t version =
   client_version < protocol_version ? 0 : protocol_version;

  const bool writable =
   get_server().writable_journal_client &&
   !get_server().client.is_pullonly();

  buffer.index = 5;
  buffer.write<int64_t>(version);
  buffer.write<int64_t>(id);
  buffer.write<int64_t>(get_server().client.get_journal_checkpoint());
  buffer.write<char>(writable ? 'W' : 'R');

  co_await boost::asio::async_write
  (
   socket,
   boost::asio::buffer(buffer.data, buffer.index),
   boost::asio::use_awaitable
  );
 }

 ///////////////////////////////////////////////////////////////////////////
 boost::asio::awaitable<void> Server::Session::run()
 ///////////////////////////////////////////////////////////////////////////
 {
  co_await handshake();

  while (true)
  {
   co_await boost::asio::async_read
   (
    socket,
    boost::asio::buffer(buffer.data, 1),
    boost::asio::use_awaitable
   );

   const char code = buffer.data[0];

   if (server.get_log_level() > 1)
   {
    const auto i = request_description.find(code);
    if (i != request_description.end())
     log(std::string("received request ") + code + ": " + i->second);
    else
     log(std::string("unknown code: ") + code);
   }

   switch (code)
   {
    case 'H': case 'I':
     co_await check_hash();
    break;

    default:
     co_return;
    break;
   }

#if 0
     case 'r':
      async_read(session, 1, 16, &Server::read_handler);
     break;

     case 'D': case 'E': case 'F': case 'G':
      lock_before_pulling = code & 1;
      send_pull_data = code & 2;
      async_read(session, 1, 16, &Server::pull_handler);
     break;

     case 'M':
      server.unlock(*this);
      write_buffer_and_next_command(session, 1);
     break;

     case 'N': case 'O':
      unlock_after_push = (code == 'O');
      push_status = code;
      async_read(session, 0, 16, &Server::push_handler);
     break;

     default:
     break;
    }
#endif
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::write_buffer_and_next_command
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::shared_ptr<Session> session,
  const size_t size
 )
 {
  boost::asio::async_write
  (
   session->socket,
   boost::asio::buffer(session->buffer.data, size),
   [this, session](std::error_code e, size_t s)
   {
    if (!e)
     read_command(session);
   }
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Server
 ////////////////////////////////////////////////////////////////////////////
 (
  Logger &logger,
  int log_level,
  int thread_count,
  std::string endpoint_path,
  Client &client,
  std::chrono::milliseconds lock_timeout
 ):
  joedb::asio::Server(logger, log_level, thread_count, std::move(endpoint_path)),
  client(client),
  writable_journal_client(dynamic_cast<Writable_Journal_Client*>(&client)),
  lock_timeout(lock_timeout),
  lock_timeout_timer(io_context),
  locked(false)
 {
  if (writable_journal_client)
   writable_journal_client->push_if_ahead();
 }
}

#undef LOGID
#undef LOG
