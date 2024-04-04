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
   out << "; session_count = " << session_count << '\n';
   out << '\n';
  });
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::lock_dequeue()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!locked && !lock_queue.empty())
  {
   locked = true;
   std::shared_ptr<Session> session = lock_queue.front();
   lock_queue.pop();

   LOGID("locking\n");

   if (lock_timeout.count() > 0)
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

   if (session->state == Session::State::waiting_for_lock)
    write_buffer_and_next_command(session, 1);
   else if (session->state == Session::State::waiting_for_lock_pull)
    pull(session);

   session->state = Session::State::locking;
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::lock(std::shared_ptr<Session> session, Session::State state)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (session->state == Session::State::not_locking)
  {
   session->state = state;
   lock_queue.push(session);
   lock_dequeue();
  }
  else
   LOG("Warning: locking an already locked session");
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
  std::shared_ptr<Session> session,
  std::error_code error
 )
 {
  if (!error)
  {
   LOGID("timeout\n");
   unlock(*session);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::finish_push
 ////////////////////////////////////////////////////////////////////////////
 (
  std::shared_ptr<Session> session,
  const char c
 )
 {
  session->buffer[0] = c;

  LOG(" done. Returning '" << c << "'\n");

  write_buffer_and_next_command(session, 1);

  if (session->unlock_after_push)
   unlock(*session);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::push_transfer_handler
 ////////////////////////////////////////////////////////////////////////////
 (
  std::shared_ptr<Session> session,
  Async_Writer writer,
  size_t remaining_size,
  std::error_code error,
  size_t bytes_transferred
 )
 {
  if (!error)
  {
   LOG('.');

   writer.write(session->buffer.data(), bytes_transferred);

   push_transfer
   (
    session,
    writer,
    remaining_size - bytes_transferred
   );
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::push_transfer
 ////////////////////////////////////////////////////////////////////////////
 (
  std::shared_ptr<Session> session,
  Async_Writer writer,
  size_t remaining_size
 )
 {
  if (remaining_size > 0)
  {
   LOG('.');

   net::async_read
   (
    session->socket,
    net::buffer
    (
     session->buffer,
     std::min(remaining_size, session->buffer.size())
    ),
    [this, session, writer, remaining_size]
    (
     std::error_code error,
     size_t bytes_transferred
    )
    {
     push_transfer_handler
     (
      session,
      writer,
      remaining_size,
      error,
      bytes_transferred
     );
    }
   );
  }
  else
  {
   client_lock->get_journal().default_checkpoint();
   client_lock->push();
   finish_push(session, 'U');
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::push_handler
 ////////////////////////////////////////////////////////////////////////////
 (
  std::shared_ptr<Session> session,
  std::error_code error,
  size_t bytes_transferred
 )
 {
  if (!error)
  {
   const int64_t start = from_network(session->buffer.data());
   const int64_t size = from_network(session->buffer.data() + 8);

   const bool conflict = (size != 0) &&
   (
    start != client.get_journal().get_checkpoint_position() ||
    (locked && session->state != Session::State::locking)
   );

   if (session->state != Session::State::locking)
   {
    LOGID("trying to push while not locking.\n");
    if (!conflict && !locked)
    {
     LOGID("OK, this session can take the lock.\n");
     lock(session, Session::State::locking);
    }
   }

   if (session->state == Session::State::locking)
    lock_timeout_timer.cancel(); // TODO: allow timeout during push

   LOGID("pushing, start = " << start << ", size = " << size << ':');

   if (conflict)
    finish_push(session, 'C');
   else if (client.is_readonly())
    finish_push(session, 'R');
   else
   {
    push_transfer
    (
     session,
     client_lock->get_journal().get_tail_writer(),
     size_t(size)
    );
   }
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::pull_transfer_handler
 ////////////////////////////////////////////////////////////////////////////
 (
  std::shared_ptr<Session> session,
  Async_Reader reader,
  std::error_code error,
  size_t bytes_transferred
 )
 {
  if (!error)
  {
   if (reader.get_remaining() > 0)
   {
    LOG('.');

    const size_t size = reader.read
    (
     session->buffer.data(),
     session->buffer.size()
    );

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
  std::shared_ptr<Session> session,
  std::error_code error,
  size_t bytes_transferred
 )
 {
  if (!error)
  {
   const int64_t checkpoint = from_network(session->buffer.data() + 1);

   if (client.is_readonly())
    client.refresh_data();

   Async_Reader reader = client.get_journal().get_tail_reader(checkpoint);
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
 void Server::pull(std::shared_ptr<Session> session)
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
  std::shared_ptr<Session> session,
  std::error_code error,
  size_t bytes_transferred
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
    readonly_journal.get_hash(checkpoint) != hash
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
 void Server::check_hash(std::shared_ptr<Session> session)
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
  std::shared_ptr<Session> session,
  std::error_code error,
  size_t bytes_transferred
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
 void Server::read_command(std::shared_ptr<Session> session)
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
  std::shared_ptr<Session> session,
  std::error_code error,
  size_t bytes_transferred
 )
 {
  if (!error)
   read_command(session);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::write_buffer_and_next_command
 ////////////////////////////////////////////////////////////////////////////
 (
  std::shared_ptr<Session> session,
  size_t size
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
 void Server::handshake(std::shared_ptr<Session> session)
 ///////////////////////////////////////////////////////////////////////////
 {
  const int64_t client_version = from_network(session->buffer.data() + 5);

  LOGID("client_version = " << client_version << '\n');

  {
   const int64_t server_version = client_version < 5 ? 0 : 6;
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
  std::shared_ptr<Session> session,
  std::error_code error,
  size_t bytes_transferred
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
  std::error_code error,
  net::ip::tcp::socket socket
 )
 {
  if (!error)
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
  acceptor.async_accept
  (
   io_context,
   [this](std::error_code error, net::ip::tcp::socket socket)
   {
    handle_accept(error, std::move(socket));
   }
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::start_interrupt_timer()
 ////////////////////////////////////////////////////////////////////////////
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

 ////////////////////////////////////////////////////////////////////////////
 void Server::handle_interrupt_timer(std::error_code error)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!error)
  {
   if (Signal::get_signal() != Signal::no_signal)
    LOG(port);

   if (Signal::get_signal() == SIGINT)
   {
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
 void Server::handle_clear_signal_timer(std::error_code error)
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
  net::io_context &io_context,
  uint16_t port,
  std::chrono::seconds lock_timeout,
  std::ostream *log_pointer
 ):
  client(client),
  client_lock(client.is_readonly() ? nullptr : new Client_Lock(client)),
  io_context(io_context),
  acceptor(io_context, net::ip::tcp::endpoint(net::ip::tcp::v4(), port)),
  port(acceptor.local_endpoint().port()),
  interrupt_timer(io_context),
  session_count(0),
  session_id(0),
  lock_timeout(lock_timeout),
  lock_timeout_timer(io_context),
  locked(false),
  log_pointer(log_pointer)
 {
  write_status();

  start_interrupt_timer();
  start_accept();

  Signal::start();
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::interrupt()
 ////////////////////////////////////////////////////////////////////////////
 {
  Signal::set_signal(SIGINT);
  io_context.post([this](){handle_interrupt_timer(std::error_code());});
  interrupt_timer.cancel();
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
