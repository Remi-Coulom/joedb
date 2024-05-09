#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/get_pid.h"
#include "joedb/io/get_time_string.h"
#include "joedb/Signal.h"
#include "joedb/Posthumous_Catcher.h"

#include <iomanip>
#include <sstream>

#define LOG(x) log([&](std::ostream &out){out << x;})
#define LOGID(x) log([&](std::ostream &out){session->write_id(out) << x;})

//#define JOEDB_SERVER_TIME_LOGGING

namespace joedb
{
#ifdef JOEDB_SERVER_TIME_LOGGING
 ////////////////////////////////////////////////////////////////////////////
 int64_t Server::get_milliseconds() const
 ////////////////////////////////////////////////////////////////////////////
 {
  return std::chrono::duration_cast<std::chrono::milliseconds>
  (
   std::chrono::steady_clock::now() - start_time
  ).count();
 }
#endif

 ////////////////////////////////////////////////////////////////////////////
 std::ostream &Server::Session::write_id(std::ostream &out) const
 ////////////////////////////////////////////////////////////////////////////
 {
#ifdef JOEDB_SERVER_TIME_LOGGING
  out << server.get_milliseconds() << ' ';
#endif

  out << server.port << '(' << id << "): ";

  return out;
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Session::Session(Server &server, net::ip::tcp::socket &&socket):
 ////////////////////////////////////////////////////////////////////////////
  id(++server.session_id),
  server(server),
  socket(std::move(socket)),
  state(not_locking)
 {
  server.log([this](std::ostream &out)
  {
   write_id(out) << "created\n";
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

   if (state == locking)
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
 void Server::write_status()
 ////////////////////////////////////////////////////////////////////////////
 {
  log([this](std::ostream &out)
  {
   out << port;
   out << "; pid = " << joedb::get_pid();
   out << ": " << get_time_string_of_now();
   out << "; sessions = " << sessions.size();
   out << "; checkpoint = ";
   out << client.get_journal().get_checkpoint_position() << '\n';
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
     LOGID("Error: locking readonly server\n");
    else
     client_lock.emplace(*push_client); // ??? takes_time
   }

   if (session->state == Session::State::waiting_for_lock_pull)
    pull(session);

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
  if (session->state == Session::State::not_locking)
  {
   session->state = state;
   lock_queue.push(session);
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

   if (client_lock && share_client && lock_queue.empty())
   {
    Posthumous_Catcher catcher;
    client_lock->set_catcher(catcher);
    client_lock.reset(); // ??? takes_time
    catcher.rethrow();
   }

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
  if (lock_timeout.count() > 0 && session->state == Session::locking)
  {
   lock_timeout_timer.expires_after(lock_timeout);
   lock_timeout_timer.async_wait
   (
    [this, session](std::error_code e)
    {
     lock_timeout_handler(session, e);
    }
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

   push_transfer(session);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::push_transfer(const std::shared_ptr<Session> session)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (session->push_remaining_size > 0)
  {
   LOG(session->push_status);

   refresh_lock_timeout(session);

   net::async_read
   (
    session->socket,
    net::buffer
    (
     session->buffer.data,
     std::min(session->push_remaining_size, session->buffer.size)
    ),
    [this, session]
    (
     std::error_code error,
     size_t bytes_transferred
    )
    {
     push_transfer_handler
     (
      session,
      error,
      bytes_transferred
     );
    }
   );
  }
  else
  {
   if (session->push_writer)
   {
    client_lock->get_journal().set_position
    (
     session->push_writer->get_position()
    );
    client_lock->get_journal().default_checkpoint();
    session->push_writer.reset();
    client_lock->push(); // ??? takes_time
   }

   session->buffer.data[0] = session->push_status;

   LOG(" done. Returning '" << session->push_status << "'\n");

   write_buffer_and_next_command(session, 1);

   if (session->unlock_after_push)
    unlock(*session);
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
    lock(session, Session::State::waiting_for_lock_push);
   }

   const bool conflict = (size != 0) &&
   (
    session->state != Session::State::locking ||
    start != client.get_journal().get_checkpoint_position()
   );

   LOGID("pushing, start = " << start << ", size = " << size << ':');

   if (is_readonly())
    session->push_status = 'R';
   else if (conflict)
    session->push_status = 'C';
   else
   {
    session->push_status = 'U';
    session->push_writer.emplace
    (
     client_lock->get_journal().get_async_tail_writer()
    );
   }

   session->push_remaining_size = size_t(size);

   push_transfer(session);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::pull_transfer_handler
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
   if (offset + reader.get_remaining() > 0)
   {
    LOG('.');

    const size_t size = reader.read // ??? takes_time
    (
     session->buffer.data + offset,
     session->buffer.size - offset
    );

    refresh_lock_timeout(session);

    net::async_write
    (
     session->socket,
     net::buffer(session->buffer.data, size + offset),
     [this, session, reader](std::error_code e, size_t s)
     {
      pull_transfer_handler(session, reader, e, s, 0);
     }
    );
   }
   else
   {
    LOG(" OK\n");
    read_command(session);
   }
  }
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
   const int64_t checkpoint = session->buffer.read<int64_t>();

   if (!client_lock) // todo: deep-share option
    client.pull(); // ??? takes_time

   const Async_Reader reader = client.get_journal().get_async_tail_reader
   (
    checkpoint
   );

   session->buffer.index = 1;
   session->buffer.write<int64_t>(reader.get_end());
   session->buffer.write<int64_t>(reader.get_remaining());

   LOGID("pulling from checkpoint = " << checkpoint << ", size = "
    << reader.get_remaining() << ':');

   pull_transfer_handler
   (
    session,
    reader,
    error,
    0,
    session->buffer.index
   );
  }
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::pull(const std::shared_ptr<Session> session)
 ///////////////////////////////////////////////////////////////////////////
 {
  net::async_read
  (
   session->socket,
   net::buffer(session->buffer.data + 1, 8),
   [this, session](std::error_code e, size_t s)
   {
    pull_handler(session, e, s);
   }
  );
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
   const int64_t checkpoint = session->buffer.read<int64_t>();
   SHA_256::Hash hash;

   for (uint32_t i = 0; i < 8; i++)
    hash[i] = session->buffer.read<uint32_t>();

   const Readonly_Journal &readonly_journal = client.get_journal();

   if
   (
    checkpoint > readonly_journal.get_checkpoint_position() ||
    readonly_journal.get_hash(checkpoint) != hash // ??? takes_time
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
 void Server::check_hash(const std::shared_ptr<Session> session)
 ///////////////////////////////////////////////////////////////////////////
 {
  net::async_read
  (
   session->socket,
   net::buffer(session->buffer.data + 1, 40),
   [this, session] (std::error_code e, size_t s)
   {
    check_hash_handler(session, e, s);
   }
  );
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
     pull(session);
    break;

    case 'L':
     lock(session, Session::State::waiting_for_lock_pull);
    break;

    case 'U': case 'p':
     session->unlock_after_push = (session->buffer.data[0] == 'U');
     net::async_read
     (
      session->socket,
      net::buffer(session->buffer.data, 16),
      [this, session](std::error_code e, size_t s)
      {
       push_handler(session, e, s);
      }
     );
    break;

    case 'u':
     if (session->state == Session::State::locking)
      unlock(*session);
     else
      session->buffer.data[0] = 't';
     write_buffer_and_next_command(session, 1);
    break;

    case 'H':
     check_hash(session);
    break;

    case 'i':
     write_buffer_and_next_command(session, 1);
    break;

    case 'Q':
     if (session->state == Session::State::locking)
      unlock(*session);
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
  net::async_read
  (
   session->socket,
   net::buffer(session->buffer.data, 1),
   [this, session](std::error_code e, size_t s)
   {
    read_command_handler(session, e, s);
   }
  );
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::write_buffer_and_next_command_handler
 ///////////////////////////////////////////////////////////////////////////
 (
  const std::shared_ptr<Session> session,
  const std::error_code error,
  const size_t bytes_transferred
 )
 {
  if (!error)
   read_command(session);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::write_buffer_and_next_command
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::shared_ptr<Session> session,
  const size_t size
 )
 {
  net::async_write
  (
   session->socket,
   net::buffer(session->buffer.data, size),
   [this, session](std::error_code e, size_t s)
   {
    write_buffer_and_next_command_handler(session, e, s);
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
    session->buffer.write<int64_t>(client_version < 10 ? 0 : server_version);
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
  net::ip::tcp::socket socket
 )
 {
  if (!error && !paused)
  {
   socket.set_option(net::ip::tcp::no_delay(true));
   std::shared_ptr<Session> session(new Session(*this, std::move(socket)));

   net::async_read
   (
    session->socket,
    net::buffer(session->buffer.data, 13),
    [this, session](std::error_code e, size_t s)
    {
     handshake_handler(session, e, s);
    }
   );

   start_accept();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::start_accept()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!paused)
  {
   acceptor.async_accept
   (
    io_context,
    [this](std::error_code error, net::ip::tcp::socket socket)
    {
     handle_accept(error, std::move(socket));
    }
   );
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::start_interrupt_timer()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!paused)
  {
   interrupt_timer.expires_after
   (
    std::chrono::seconds(interrupt_check_seconds)
   );

   interrupt_timer.async_wait
   (
    [this](std::error_code e)
    {
     handle_interrupt_timer(e);
    }
   );
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::handle_interrupt_timer(const std::error_code error)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!error && !paused)
  {
   if (Signal::get_signal() != Signal::no_signal)
    LOG(port);

   if (Signal::get_signal() == SIGINT)
   {
    paused = true;
    LOG(": Received SIGINT, interrupting.\n");
    for (Session *session: sessions)
     session->socket.close();
    acceptor.cancel();
   }
   else
   {
    if (Signal::get_signal() == SIGUSR1)
    {
     log([this](std::ostream &out)
     {
      out << ": timeout = " << lock_timeout.count();
      out << "s. Received SIGUSR1, listing sessions. Count = ";
      out << sessions.size() << ".\n";

      for (const Session *session: sessions)
      {
       out << ' ' << session->id;
       out << ": state = " << session->state;
       out << "; remote_endpoint = " << session->socket.remote_endpoint();
       out << '\n';
      }
     });
    }
    else if (Signal::get_signal() == SIGUSR2)
    {
     LOG("; Received SIGUSR2\n");
     write_status();
    }

    if (Signal::get_signal() == Signal::no_signal)
    {
     start_interrupt_timer();
    }
    else
    {
     interrupt_timer.expires_after
     (
      std::chrono::seconds(clear_signal_seconds)
     );

     interrupt_timer.async_wait
     (
      [this](std::error_code e)
      {
       handle_clear_signal_timer(e);
      }
     );
    }
   }
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::handle_clear_signal_timer(const std::error_code error)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!error)
  {
   Signal::set_signal(Signal::no_signal);
   start_interrupt_timer();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Server
 ////////////////////////////////////////////////////////////////////////////
 (
  Pullonly_Client &client,
  const bool share_client,
  net::io_context &io_context,
  const uint16_t port,
  const std::chrono::seconds lock_timeout,
  std::ostream * const log_pointer
 ):
  start_time(std::chrono::steady_clock::now()),
  client(client),
  push_client(client.get_push_client()),
  share_client(share_client),
  io_context(io_context),
  acceptor(io_context, net::ip::tcp::endpoint(net::ip::tcp::v4(), port)),
  port(acceptor.local_endpoint().port()),
  interrupt_timer(io_context),
  paused(false),
  session_id(0),
  lock_timeout(lock_timeout),
  lock_timeout_timer(io_context),
  locked(false),
  log_pointer(log_pointer)
 {
  LOG(port << ": constructor\n");

  if (push_client)
   push_client->push_unlock();

  if (!share_client && !is_readonly())
   client_lock.emplace(*push_client);
  else
   client.pull();

  write_status();

  Signal::start();
  start_interrupt_timer();
  start_accept();
 }

 ////////////////////////////////////////////////////////////////////////////
 bool Server::is_readonly() const
 ////////////////////////////////////////////////////////////////////////////
 {
  return client.is_readonly() || !push_client;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::set_log(std::ostream *new_log)
 ////////////////////////////////////////////////////////////////////////////
 {
  io_context.post
  (
   [this, new_log]()
   {
    log_pointer = new_log;
   }
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::pause()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!io_context.stopped())
  {
   io_context.post
   (
    [this]()
    {
     LOG(port << ": pause. sessions.size() = " << sessions.size() << '\n');
     paused = true;
     acceptor.cancel();
     interrupt_timer.cancel();
    }
   );
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::restart()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (io_context.stopped())
  {
   io_context.restart();
   paused = false;
   start_interrupt_timer();
   start_accept();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::send_signal(int status)
 ////////////////////////////////////////////////////////////////////////////
 {
  Signal::set_signal(status);
  io_context.post([this](){handle_interrupt_timer(std::error_code());});
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
