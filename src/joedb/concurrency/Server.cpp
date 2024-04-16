#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/network_integers.h"
#include "joedb/concurrency/get_pid.h"
#include "joedb/io/get_time_string.h"
#include "joedb/Signal.h"

#include <iomanip>
#include <sstream>

#define LOG(x) log([&](std::ostream &out){out << x;})
#define LOGID(x) log([&](std::ostream &out){session->write_id(out) << x;})

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 std::ostream &Server::Session::write_id(std::ostream &out) const
 ////////////////////////////////////////////////////////////////////////////
 {
  out << server.port << '(' << id << "): ";

  return out;
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Session::Session(Server &server, net::ip::tcp::socket && socket):
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
  ++server.session_count;
  server.sessions.insert(this);
  server.write_status();
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Session::~Session()
 ////////////////////////////////////////////////////////////////////////////
 {
  try
  {
   --server.session_count;
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
   out << '\n';
   out << "port = " << port;
   out << "; pid = " << joedb::get_pid();
   out << ": " << get_time_string_of_now();
   out << "; sessions = " << session_count;
   out << "; cp = ";
   out << client.get_journal().get_checkpoint_position() << '\n';
   out << '\n';
  });
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::lock_dequeue()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!locked && !lock_queue.empty())
  {
   if (!client_lock)
    client_lock.emplace(client); // ??? async

   locked = true;
   const std::shared_ptr<Session> session = lock_queue.front();
   lock_queue.pop();

   LOGID("locking\n");

   if (session->state == Session::State::waiting_for_lock)
    write_buffer_and_next_command(session, 1);
   else if (session->state == Session::State::waiting_for_lock_pull)
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
  {
   LOGID("Warning: locking an already locked session\n");
   write_buffer_and_next_command(session, 1);
  }
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

   if (share_client && lock_queue.empty())
    client_lock.reset(); // ??? async

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
    session->push_writer->write(session->buffer.data(), bytes_transferred); // ??? async

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
     session->buffer,
     std::min(session->push_remaining_size, session->buffer.size())
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
    client_lock->push(); // ??? async
   }

   session->buffer[0] = session->push_status;

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
   const int64_t start = from_network(session->buffer.data());
   const int64_t size = from_network(session->buffer.data() + 8);

   if (locked && session->state != Session::State::locking)
   {
    LOGID("trying to push while someone else is locking\n");
   }
   else if (!locked)
   {
    LOGID("Taking the lock for push attempt.\n");
    lock(session, Session::State::locking);
   }

   const bool conflict = (size != 0) &&
   (
    session->state != Session::State::locking ||
    start != client.get_journal().get_checkpoint_position()
   );

   LOGID("pushing, start = " << start << ", size = " << size << ':');

   if (conflict)
    session->push_status = 'C';
   else if (client.is_readonly())
    session->push_status = 'R';
   else
   {
    session->push_status = 'U';
    session->push_writer.emplace
    (
     client_lock->get_journal().get_async_tail_writer()
    );
   }

   session->push_remaining_size = size;

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
  const size_t bytes_transferred
 )
 {
  if (!error)
  {
   if (reader.get_remaining() > 0)
   {
    LOG('.');

    const size_t size = reader.read // ??? async
    (
     session->buffer.data(),
     session->buffer.size()
    );

    refresh_lock_timeout(session);

    net::async_write
    (
     session->socket,
     net::buffer(session->buffer, size_t(size)),
     [this, session, reader](std::error_code e, size_t s)
     {
      pull_transfer_handler(session, reader, e, s);
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
   const int64_t checkpoint = from_network(session->buffer.data() + 1);

   if (client.is_readonly())
    client.refresh_data();
   else if (!client_lock) // todo: deep-share option
    client.pull(); // ??? async

   const Async_Reader reader = client.get_journal().get_async_tail_reader
   (
    checkpoint
   );

   to_network(reader.get_remaining(), session->buffer.data() + 9);

   LOGID("pulling from checkpoint = " << checkpoint << ", size = "
    << reader.get_remaining() << ':');

   net::async_write
   (
    session->socket,
    net::buffer(session->buffer, 17),
    [this, session, reader](std::error_code e, size_t s)
    {
     pull_transfer_handler(session, reader, e, s);
    }
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
   net::buffer(session->buffer.data() + 1, 8),
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
   const int64_t checkpoint = from_network(session->buffer.data() + 1);
   SHA_256::Hash hash;

   for (uint32_t i = 0; i < 8; i++)
    hash[i] = uint32_from_network(session->buffer.data() + 9 + 4 * i);

   const Readonly_Journal &readonly_journal = client.get_journal();

   if
   (
    checkpoint > readonly_journal.get_checkpoint_position() ||
    readonly_journal.get_hash(checkpoint) != hash // ??? async
   )
   {
    session->buffer[0] = 'h';
   }

   LOGID("hash for checkpoint = " << checkpoint << ", result = "
    << session->buffer[0] << '\n');

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
   net::buffer(session->buffer.data() + 1, 40),
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
   LOGID(session->buffer[0] << '\n');

   switch (session->buffer[0])
   {
    case 'P':
     pull(session);
    break;

    case 'L':
     lock(session, Session::State::waiting_for_lock_pull);
    break;

    case 'U': case 'p':
     session->unlock_after_push = (session->buffer[0] == 'U');
     net::async_read
     (
      session->socket,
      net::buffer(session->buffer.data(), 16),
      [this, session](std::error_code e, size_t s)
      {
       push_handler(session, e, s);
      }
     );
    break;

    case 'l':
     lock(session, Session::State::waiting_for_lock);
    break;

    case 'u':
     if (session->state == Session::State::locking)
      unlock(*session);
     else
      session->buffer[0] = 't';
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
   net::buffer(session->buffer, 1),
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
   net::buffer(session->buffer, size),
   [this, session](std::error_code e, size_t s)
   {
    write_buffer_and_next_command_handler(session, e, s);
   }
  );
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::handshake(const std::shared_ptr<Session> session)
 ///////////////////////////////////////////////////////////////////////////
 {
  const int64_t client_version = from_network(session->buffer.data() + 5);

  LOGID("client_version = " << client_version << '\n');

  {
   const int64_t server_version = client_version < 5 ? 0 : 7;
   to_network(server_version, session->buffer.data() + 5);
  }

  to_network(session->id, session->buffer.data() + 5 + 8);
  to_network
  (
   client.get_journal().get_checkpoint_position(),
   session->buffer.data() + 5 + 8 + 8
  );

  write_buffer_and_next_command(session, 5 + 8 + 8 + 8);
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
   if
   (
    session->buffer[0] == 'j' &&
    session->buffer[1] == 'o' &&
    session->buffer[2] == 'e' &&
    session->buffer[3] == 'd' &&
    session->buffer[4] == 'b'
   )
   {
    handshake(session);
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
   std::shared_ptr<Session> session(new Session(*this, std::move(socket)));

   net::async_read
   (
    session->socket,
    net::buffer(session->buffer, 13),
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
      out << session_count << ".\n";

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
  Client &client,
  const bool share_client,
  net::io_context &io_context,
  const uint16_t port,
  const std::chrono::seconds lock_timeout,
  std::ostream * const log_pointer
 ):
  client(client),
  share_client(share_client),
  io_context(io_context),
  acceptor(io_context, net::ip::tcp::endpoint(net::ip::tcp::v4(), port)),
  port(acceptor.local_endpoint().port()),
  interrupt_timer(io_context),
  paused(false),
  session_count(0),
  session_id(0),
  lock_timeout(lock_timeout),
  lock_timeout_timer(io_context),
  locked(false),
  log_pointer(log_pointer)
 {
  LOG("Server::Server\n");

  if (client.get_checkpoint_difference() > 0)
  {
   LOG("Server::Server: pushing to connection\n");
   client.push_unlock();
  }

  if (!client.is_readonly())
  {
   if (share_client)
    client.pull();
   else
    client_lock.emplace(client);
  }

  write_status();

  Signal::start();
  start_interrupt_timer();
  start_accept();
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
 Server::~Server()
 ////////////////////////////////////////////////////////////////////////////
 {
  try
  {
   if (this->session_count > 0)
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
