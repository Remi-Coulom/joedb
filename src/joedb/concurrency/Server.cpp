#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Client.h"
#include "joedb/get_pid.h"
#include "joedb/concurrency/protocol_version.h"
#include "joedb/ui/get_time_string.h"
#include "joedb/journal/File_Hasher.h"

#define LOG(x) log([&](std::ostream &out){out << x;})
#define LOGID(x) log([&](std::ostream &out){session->write_id(out) << x;})

#include <asio/read.hpp>
#include <asio/write.hpp>

namespace joedb
{
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

  out << server.port << '(' << id << "): ";

  return out;
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Session::Session(Server &server, asio::ip::tcp::socket &&socket):
 ////////////////////////////////////////////////////////////////////////////
  id(++server.session_id),
  server(server),
  socket(std::move(socket)),
  state(State::not_locking)
 {
  server.log([this](std::ostream &out)
  {
   write_id(out) << "created (remote endpoint: ";
   out << this->socket.remote_endpoint() << ")\n";
  });
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
  asio::async_read
  (
   session->socket,
   asio::buffer(session->buffer.data + offset, size),
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
   out << port;
   out << ": pid = " << joedb::get_pid();
   out << "; " << get_time_string_of_now();
   out << "; sessions = " << sessions.size();
   out << "; checkpoint = ";
   out << client.get_checkpoint() << '\n';
  });
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::lock_dequeue()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!locked && !lock_queue.empty())
  {
   locked = true;
   const std::shared_ptr<Session> session = lock_queue.front();
   lock_queue.pop();
   LOGID("locking\n");

   if (!client_lock)
   {
    if (is_readonly())
    {
     LOGID("Error: locking pull-only server\n");
     session->buffer.data[0] = 'R';
    }
    else
    {
     JOEDB_ASSERT(push_client);
     client_lock.emplace(*push_client); // ??? takes_time
    }
   }

   if (session->state == Session::State::waiting_for_lock_to_pull)
    start_pulling(session);

   session->state = Session::State::locking;
   refresh_lock_timeout(session);
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
   LOGID("Error: locking an already locked session\n");
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

   session->push_remaining_size -= bytes_transferred;

   if (session->progress_bar)
   {
    session->progress_bar->print_remaining
    (
     int64_t(session->push_remaining_size)
    );
   }

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
    std::min(session->push_remaining_size, session->buffer.size),
    &Server::push_transfer_handler
   );
  }
  else
  {
   if (session->push_writer)
   {
    if (client_lock)
    {
     client_lock->get_journal().set_position
     (
      session->push_writer->get_position()
     );
     client_lock->get_journal().default_checkpoint();

     // ??? takes_time
     if (share_client && session->unlock_after_push)
     {
      client_lock->push_unlock();
      client_lock.reset();
     }
     else
      client_lock->push();
    }

    session->push_writer.reset();
   }

   session->buffer.data[0] = session->push_status;

   if (session->progress_bar)
    session->progress_bar.reset();
   else
    LOG(" done. ");

   LOG("Returning '" << session->push_status << "'\n");

   write_buffer_and_next_command(session, 1);

   if (session->unlock_after_push)
    unlock(*session);

   for (auto *other_session: sessions)
   {
    if
    (
     other_session->state == Session::State::waiting_for_push_to_pull &&
     other_session->pull_checkpoint < client.get_checkpoint()
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
   const int64_t start = session->buffer.read<int64_t>();
   const int64_t size = session->buffer.read<int64_t>();

   if (locked && session->state != Session::State::locking)
   {
    LOGID("trying to push while someone else is locking\n");
   }
   else if (!locked)
   {
    LOGID("Taking the lock for push attempt.\n");
    lock(session, Session::State::waiting_for_lock_to_push);
   }

   const bool conflict = (size != 0) &&
   (
    session->state != Session::State::locking ||
    start != client.get_journal().get_checkpoint_position()
   );

   LOGID("pushing, start = " << start << ", size = " << size);

   if (log_pointer && size > session->buffer.ssize)
    session->progress_bar.emplace(size, *log_pointer);

   if (is_readonly())
    session->push_status = 'R';
   else if (conflict)
    session->push_status = 'C';
   else
   {
    session->push_status = 'U';
    if (client_lock)
    {
     session->push_writer.emplace
     (
      client_lock->get_journal().get_async_tail_writer()
     );
    }
   }

   session->push_remaining_size = size_t(size);

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
   if (session->progress_bar)
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

     asio::async_write
     (
      session->socket,
      asio::buffer(session->buffer.data, size + offset),
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
    if (session->progress_bar)
     session->progress_bar.reset();
    else
     LOG(" OK\n");
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
  session->buffer.write<int64_t>(reader.get_remaining());

  LOGID("reading from = " << reader.get_current() << ", size = "
   << reader.get_remaining() << ':');

  if (log_pointer && reader.get_remaining() > session->buffer.ssize)
   session->progress_bar.emplace(reader.get_remaining(), *log_pointer);

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

  session->buffer.index = 1;
  session->buffer.write<int64_t>(client.get_checkpoint());

  if (session->send_pull_data)
  {
   start_reading
   (
    session,
    client.get_journal().get_async_tail_reader(session->pull_checkpoint)
   );
  }
  else
   write_buffer_and_next_command(session, session->buffer.index);
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
   session->pull_checkpoint = session->buffer.read<int64_t>();
   const std::chrono::milliseconds wait{session->buffer.read<int64_t>()};

   if (!client_lock) // todo: deep-share option
    client.pull(); // ??? takes_time

   if (wait.count() > 0 && session->pull_checkpoint == client.get_checkpoint())
   {
    LOGID
    (
     "waiting at checkpoint = " << session->pull_checkpoint <<
     " for " << wait.count() << " milliseconds\n"
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
 void Server::pull
 ///////////////////////////////////////////////////////////////////////////
 (
  const std::shared_ptr<Session> session,
  bool lock,
  bool send
 )
 {
  session->lock_before_pulling = lock;
  session->send_pull_data = send;
  async_read(session, 1, 16, &Server::pull_handler);
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
   int64_t offset = session->buffer.read<int64_t>();
   int64_t size = session->buffer.read<int64_t>();
   int64_t until = offset + size;

   if (until > client.get_checkpoint())
    until = client.get_checkpoint();
   if (offset > until)
    offset = until;

   const Async_Reader reader = client.get_journal().get_async_reader
   (
    offset,
    until
   );

   session->buffer.index = 1;
   start_reading(session, reader);
  }
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::read_blob_handler
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
   const int64_t blob_position = session->buffer.read<int64_t>();
   const Async_Reader reader = client.get_journal().get_async_blob_reader
   (
    Blob(blob_position)
   );

   session->buffer.index = 1;
   start_reading(session, reader);
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

   const Readonly_Journal &readonly_journal = client.get_journal();

   if
   (
    checkpoint > readonly_journal.get_checkpoint_position() ||
    Journal_Hasher::get_hash(readonly_journal, checkpoint) != hash // ??? takes_time
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
   LOGID(session->buffer.data[0] << '\n');

   switch (session->buffer.data[0])
   {
    case 'P':
     pull(session, false, true);
    break;

    case 'L':
     pull(session, true, true);
    break;

    case 'i':
     pull(session, false, false);
    break;

    case 'l':
     pull(session, true, false);
    break;

    case 'U': case 'p':
     session->unlock_after_push = (session->buffer.data[0] == 'U');
     async_read(session, 0, 16, &Server::push_handler);
    break;

    case 'u':
     if (session->state == Session::State::locking)
      unlock(*session);
     else
      session->buffer.data[0] = 't';
     write_buffer_and_next_command(session, 1);
    break;

    case 'H':
     async_read(session, 1, 40, &Server::check_hash_handler);
    break;

    case 'r':
     async_read(session, 1, 16, &Server::read_handler);
    break;

    case 'b':
     async_read(session, 1, 8, &Server::read_blob_handler);
    break;

    default:
     LOGID("unexpected command\n");
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
  asio::async_write
  (
   session->socket,
   asio::buffer(session->buffer.data, size),
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

   if
   (
    session->buffer.read<char>() == 'j' &&
    session->buffer.read<char>() == 'o' &&
    session->buffer.read<char>() == 'e' &&
    session->buffer.read<char>() == 'd' &&
    session->buffer.read<char>() == 'b'
   )
   {
    const int64_t client_version = session->buffer.read<int64_t>();
    LOGID("client_version = " << client_version << '\n');

    session->buffer.index = 5;
    session->buffer.write<int64_t>(client_version < protocol_version ? 0 : protocol_version);
    session->buffer.write<int64_t>(session->id);
    session->buffer.write<int64_t>(client.get_checkpoint());
    session->buffer.write<char>(is_readonly() ? 'R' : 'W');

    write_buffer_and_next_command(session, session->buffer.index);
    return;
   }

   LOGID("bad handshake\n");
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::handle_accept
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::error_code error,
  asio::ip::tcp::socket socket
 )
 {
  if (!error && !stopped)
  {
   socket.set_option(asio::ip::tcp::no_delay(true));
   std::shared_ptr<Session> session(new Session(*this, std::move(socket)));
   async_read(session, 0, 13, &Server::handshake_handler);

   start_accept();
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
    [this](std::error_code error, asio::ip::tcp::socket socket)
    {
     handle_accept(error, std::move(socket));
    }
   );
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Server
 ////////////////////////////////////////////////////////////////////////////
 (
  Client &client,
  const bool share_client,
  asio::io_context &io_context,
  const uint16_t port,
  const std::chrono::milliseconds lock_timeout,
  std::ostream * const log_pointer
 ):
  start_time(std::chrono::steady_clock::now()),
  client(client),
  push_client(dynamic_cast<Writable_Journal_Client*>(&client)),
  share_client(share_client),
  io_context(io_context),
  acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
  port(acceptor.local_endpoint().port()),
  stopped(true),
  interrupt_signals(io_context, SIGINT, SIGTERM),
  session_id(0),
  lock_timeout(lock_timeout),
  lock_timeout_timer(io_context),
  locked(false),
  log_pointer(log_pointer)
 {
  if (push_client)
   push_client->push_unlock();

  if (!share_client && !is_readonly())
  {
   JOEDB_ASSERT(push_client);
   client_lock.emplace(*push_client);
  }
  else
   client.pull();

  start();
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::start()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (stopped)
  {
   stopped = false;

   interrupt_signals.async_wait([this](const asio::error_code &error, int)
   {
    if (!error)
     stop();
   });

   start_accept();

   // Note: C++20 has operator<< for durations
   LOG
   (
    port <<
    ": start. lock_timeout = " << lock_timeout.count() <<
    "; protocol_version = " << protocol_version << '\n'
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
   LOG(port << ": stop\n");

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
   if (!this->sessions.empty())
   {
    LOG("Bug: destroying server before sessions.\n");
   }
  }
  catch (...)
  {
  }
 }
}

#undef LOGID
#undef LOG
