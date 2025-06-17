#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Client.h"
#include "joedb/get_pid.h"
#include "joedb/concurrency/protocol_version.h"
#include "joedb/ui/get_time_string.h"
#include "joedb/journal/File_Hasher.h"

#define LOG(x) log([&](std::ostream &out){out << x;})
#define LOGID(x) log([&](std::ostream &out){session->write_id(out) << x;})

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
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
 std::chrono::milliseconds Server::get_time_stamp() const
 ////////////////////////////////////////////////////////////////////////////
 {
  return std::chrono::duration_cast<std::chrono::milliseconds>
  (
   std::chrono::steady_clock::now() - start_time
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 std::ostream &Server::Session::write_id(std::ostream &out) const
 ////////////////////////////////////////////////////////////////////////////
 {
#if 0
  out << server.get_time_stamp().count() << ' ';
#endif

  out << server.endpoint_path << '('
      << server.client.get_journal_checkpoint() << "): " << id << ": ";

  return out;
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Session::Session
 ////////////////////////////////////////////////////////////////////////////
 (
  Server &server,
  boost::asio::local::stream_protocol::socket &&socket
 ):
  id(++server.session_id),
  server(server),
  socket(std::move(socket)),
  state(State::not_locking)
 {
  server.log([this](std::ostream &out)
  {
   write_id(out) << "new session created\n";}
  );
  server.sessions.insert(this);
  server.write_status();
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Session::~Session()
 ////////////////////////////////////////////////////////////////////////////
 {
  try
  {
   server.sessions.erase(this);

   if (state == State::locking)
   {
    server.log([this](std::ostream &out)
    {
     write_id(out) << "removing lock held by dying session.\n";
    });

    server.unlock(*this);
   }

   server.log([this](std::ostream &out)
   {
    write_id(out) << "deleted\n";
   });

   server.write_status();
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
 void Server::write_status()
 ////////////////////////////////////////////////////////////////////////////
 {
  log([this](std::ostream &out)
  {
   out << endpoint_path << '(' << client.get_journal_checkpoint();
   out << "): pid = " << joedb::get_pid();
   out << "; " << get_time_string_of_now();
   out << "; sessions = " << sessions.size() << '\n';
  });
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
    LOGID("locking\n");

    if (!client_lock)
    {
     if (!writable_journal_client)
     {
      LOGID("error: locking pull-only server\n");
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
   LOGID("error: locking an already locked session\n");
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::unlock(Session &session)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (session.state == Session::State::locking)
  {
   log([&session](std::ostream &out)
   {
    session.write_id(out) << "unlocking\n";
   });
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
   LOGID("timeout\n");

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
   LOGID("returning " << session->push_status << '\n');
   write_buffer_and_next_command(session, 1);

   if (session->unlock_after_push)
    unlock(*session);

   for (auto *other_session: sessions)
   {
    if
    (
     other_session->state == Session::State::waiting_for_push_to_pull &&
     other_session->pull_checkpoint < client.get_journal_checkpoint()
    )
    {
     other_session->state = Session::State::not_locking;
     start_pulling(other_session->shared_from_this());
    }
   }
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
    LOGID("trying to push while someone else is locking\n");
   }
   else if (!locked)
   {
    LOGID("taking the lock for push attempt.\n");
    lock(session, Session::State::waiting_for_lock_to_push);
   }

   const bool conflict =
   (
    session->state != Session::State::locking ||
    from != client.get_journal().get_checkpoint()
   );

   LOGID("pushing, from = " << from << ", until = " << until);
   session->progress_bar.emplace(until - from, log_pointer);

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
     LOG("error: unexpected end of file\n");
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
  LOGID("reading from = " << reader.get_current() << ", until = " << reader.get_end());
  session->progress_bar.emplace(reader.get_remaining(), log_pointer);

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
    LOGID
    (
     "waiting at checkpoint = " << session->pull_checkpoint <<
     " for " << double(wait.count()) * 0.001 << "s\n"
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
 void Server::check_hash_handler
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
   const auto checkpoint = session->buffer.read<int64_t>();
   const auto hash = session->buffer.read<SHA_256::Hash>();

   const Readonly_Journal &journal = client.get_journal();

   if
   (
    checkpoint > journal.get_checkpoint() ||
    hash != (session->buffer.data[0] == 'H' // ??? takes_time
    ? Journal_Hasher::get_fast_hash(journal, checkpoint)
    : Journal_Hasher::get_full_hash(journal, checkpoint))
   )
   {
    session->buffer.data[0] = 'h';
   }

   LOGID("hash for checkpoint = " << checkpoint << ", result = "
    << session->buffer.data[0] << '\n');

   write_buffer_and_next_command(session, 1);
  }
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::read_command_handler
 ///////////////////////////////////////////////////////////////////////////
 (
  const std::shared_ptr<Session> session,
  const std::error_code error,
  const size_t bytes_transferred
 )
 {
  if (!error)
  {
   const char code = session->buffer.data[0];

   if (log_pointer)
   {
    const auto i = request_description.find(code);
    if (i != request_description.end())
     LOGID("received request " << code << ": " << i->second << '\n');
    else
     LOGID("unknown code: " << code << '\n');
   }

   switch (code)
   {
    case 'H': case 'I':
     async_read(session, 1, 40, &Server::check_hash_handler);
    break;

    case 'r':
     async_read(session, 1, 16, &Server::read_handler);
    break;

    case 'D': case 'E': case 'F': case 'G':
     session->lock_before_pulling = code & 1;
     session->send_pull_data = code & 2;
     async_read(session, 1, 16, &Server::pull_handler);
    break;

    case 'M':
     unlock(*session);
     write_buffer_and_next_command(session, 1);
    break;

    case 'N': case 'O':
     session->unlock_after_push = (code == 'O');
     session->push_status = code;
     async_read(session, 0, 16, &Server::push_handler);
    break;

    default:
    break;
   }
  }
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::read_command(const std::shared_ptr<Session> session)
 ///////////////////////////////////////////////////////////////////////////
 {
  async_read(session, 0, 1, &Server::read_command_handler);
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

 ///////////////////////////////////////////////////////////////////////////
 void Server::handshake_handler
 ///////////////////////////////////////////////////////////////////////////
 (
  const std::shared_ptr<Session> session,
  const std::error_code error,
  const size_t bytes_transferred
 )
 {
  if (!error)
  {
   session->buffer.index = 0;

   if (session->buffer.read<std::array<char, 5>>() == Header::joedb)
   {
    const int64_t client_version = session->buffer.read<int64_t>();
    LOGID("client_version = " << client_version << '\n');

    session->buffer.index = 5;
    session->buffer.write<int64_t>(client_version < protocol_version ? 0 : protocol_version);
    session->buffer.write<int64_t>(session->id);
    session->buffer.write<int64_t>(client.get_journal_checkpoint());
    session->buffer.write<char>
    (
     (writable_journal_client && !client.is_pullonly()) ? 'W' : 'R'
    );

    write_buffer_and_next_command(session, session->buffer.index);
    return;
   }

   LOGID("bad handshake\n");
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::start_accept()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!stopped)
  {
   acceptor.async_accept
   (
    io_context,
    [this](std::error_code error, boost::asio::local::stream_protocol::socket socket)
    {
     if (!error && !stopped)
     {
      std::shared_ptr<Session> session(new Session(*this, std::move(socket)));
      async_read(session, 0, 13, &Server::handshake_handler);

      start_accept();
     }
    }
   );
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Server
 ////////////////////////////////////////////////////////////////////////////
 (
  Client &client,
  boost::asio::io_context &io_context,
  std::string endpoint_path,
  const std::chrono::milliseconds lock_timeout,
  std::ostream * const log_pointer
 ):
  start_time(std::chrono::steady_clock::now()),
  client(client),
  writable_journal_client(dynamic_cast<Writable_Journal_Client*>(&client)),
  io_context(io_context),
  endpoint_path(std::move(endpoint_path)),
  endpoint(this->endpoint_path),
  acceptor(io_context, endpoint, false),
  stopped(true),
  interrupt_signals(io_context, SIGINT, SIGTERM),
  session_id(0),
  lock_timeout(lock_timeout),
  lock_timeout_timer(io_context),
  locked(false),
  log_pointer(log_pointer)
 {
  if (writable_journal_client)
   writable_journal_client->push_if_ahead();

  start();
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::start()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (stopped)
  {
   stopped = false;

   if (!client.is_shared() && writable_journal_client)
    client_lock.emplace(*writable_journal_client);

   interrupt_signals.async_wait([this](const boost::system::error_code &error, int)
   {
    if (!error)
     stop();
   });

   start_accept();

   // Note: C++20 has operator<< for durations
   LOG
   (
    get_endpoint_path() <<
    ": start. lock_timeout = " << double(lock_timeout.count()) * 0.001 <<
    "s; protocol_version = " << protocol_version <<
    "; shared = " << client.is_shared() << '\n'
   );
   write_status();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::stop_after_sessions()
 ////////////////////////////////////////////////////////////////////////////
 {
  acceptor.cancel();
  interrupt_signals.cancel();
  stopped = true;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::stop()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!stopped)
  {
   LOG(get_endpoint_path() << ": stop\n");

   for (Session *session: sessions)
   {
    session->socket.close();
    session->pull_timer.reset();
   }

   if (client_lock)
   {
    client_lock->unlock();
    client_lock.reset();
   }

   stop_after_sessions();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::~Server()
 ////////////////////////////////////////////////////////////////////////////
 {
  try
  {
   if (!sessions.empty())
    LOG("Destroying server before sessions.\n");
   std::remove(endpoint_path.c_str());
  }
  catch (...)
  {
  }
 }
}

#undef LOGID
#undef LOG
