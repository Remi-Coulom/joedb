#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/network_integers.h"
#include "joedb/io/Dump_Writable.h"

#include <iostream>
#include <iomanip>
#include <functional>
#include <csignal>
#include <sstream>

namespace joedb
{
 std::atomic<bool> Server::interrupted(false);

 ////////////////////////////////////////////////////////////////////////////
 void CDECL Server::signal_handler(int sig)
 ////////////////////////////////////////////////////////////////////////////
 {
  interrupted = true;
 }

 ////////////////////////////////////////////////////////////////////////////
 std::ostream &Server::Session::write_id(std::ostream &out)
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
  write_id(std::cerr) << "created\n";
  ++server.session_count;
  server.write_status();
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Session::~Session()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (state == locking)
  {
   write_id(std::cerr) << "removing lock held by dying session.\n";
   server.unlock(*this);
  }
  write_id(std::cerr) << "deleted\n";
  --server.session_count;
  server.write_status();
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::write_status()
 ////////////////////////////////////////////////////////////////////////////
 {
  std::cerr << '\n';
  std::cerr << joedb::Dump_Writable::get_local_time(std::time(nullptr));
  std::cerr << "; port = " << port;
  std::cerr << "; session_count = " << session_count << '\n';
  std::cerr << '\n';
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

   session->write_id(std::cerr) << "locking\n";

   if (lock_timeout_seconds > 0)
   {
    lock_timeout_timer.expires_after
    (
     std::chrono::seconds(lock_timeout_seconds)
    );

    lock_timeout_timer.async_wait
    (
     std::bind
     (
      &Server::lock_timeout_handler,
      this,
      session,
      std::placeholders::_1
     )
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
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::unlock(Session &session)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (session.state == Session::State::locking)
  {
   session.write_id(std::cerr) << "unlocking\n";
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
   session->write_id(std::cerr) << "timeout\n";
   unlock(*session);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::push_transfer_handler
 ////////////////////////////////////////////////////////////////////////////
 (
  std::shared_ptr<Session> session,
  size_t offset,
  size_t remaining_size,
  bool conflict,
  std::error_code error,
  size_t bytes_transferred
 )
 {
  if (!error)
  {
   std::cerr << '.';

   push_transfer
   (
    session,
    offset + bytes_transferred,
    remaining_size - bytes_transferred,
    conflict
   );
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::push_transfer
 ////////////////////////////////////////////////////////////////////////////
 (
  std::shared_ptr<Session> session,
  size_t offset,
  size_t remaining_size,
  bool conflict
 )
 {
  if (remaining_size > 0)
  {
   std::cerr << '.';

   net::async_read
   (
    session->socket,
    net::buffer(&push_buffer[offset], remaining_size),
    [this, session, offset, remaining_size, conflict]
    (
     std::error_code error,
     size_t bytes_transferred
    ) mutable
    {
     push_transfer_handler
     (
      session,
      offset,
      remaining_size,
      conflict,
      error,
      bytes_transferred
     );
    }
   );
  }
  else
  {
   if (conflict)
    session->buffer[0] = 'C';
   else
   {
    journal.append_raw_tail(push_buffer.data(), offset);
    session->buffer[0] = 'U';
   }

   std::cerr << " done. Returning '" << session->buffer[0] << "'\n";

   write_buffer_and_next_command(session, 1);

   unlock(*session);
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
   const int64_t start = from_network(session->buffer);
   const int64_t size = from_network(session->buffer + 8);

   const bool conflict = (size != 0) &&
   (
    start != journal.get_checkpoint_position() ||
    (locked && session->state != Session::State::locking)
   );

   if (session->state != Session::State::locking)
   {
    session->write_id(std::cerr) << "trying to push while not locking.\n";
    if (!conflict && !locked)
    {
     session->write_id(std::cerr) << "OK, this session can take the lock.\n";
     lock(session, Session::State::locking);
    }
   }

   if (session->state == Session::State::locking)
    lock_timeout_timer.cancel();

   if (!conflict && size > int64_t(push_buffer.size()))
    push_buffer.resize(size_t(size));

   session->write_id(std::cerr) << "pushing, start = ";
   std::cerr << start << ", size = " << size << ':';

   push_transfer
   (
    session,
    0,
    size_t(size),
    conflict
   );
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
    std::cerr << '.';

    const size_t size = reader.read
    (
     session->buffer,
     Session::buffer_size
    );

    net::async_write
    (
     session->socket,
     net::buffer(session->buffer, size_t(size)),
     std::bind
     (
      &Server::pull_transfer_handler,
      this,
      session,
      reader,
      std::placeholders::_1,
      std::placeholders::_2
     )
    );
   }
   else
   {
    std::cerr << " OK\n";
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
   const int64_t checkpoint = from_network(session->buffer + 1);

   Async_Reader reader = journal.get_tail_reader(checkpoint);
   to_network(reader.get_remaining(), session->buffer + 9);

   session->write_id(std::cerr) << "pulling from checkpoint = ";
   std::cerr << checkpoint << ", size = " << reader.get_remaining() << ':';

   net::async_write
   (
    session->socket,
    net::buffer(session->buffer, 17),
    std::bind
    (
     &Server::pull_transfer_handler,
     this,
     session,
     reader,
     std::placeholders::_1,
     std::placeholders::_2
    )
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
   net::buffer(session->buffer + 1, 8),
   std::bind
   (
    &Server::pull_handler,
    this,
    session,
    std::placeholders::_1,
    std::placeholders::_2
   )
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
   session->write_id(std::cerr) << session->buffer[0] << '\n';

   switch (session->buffer[0])
   {
    case 'P':
     pull(session);
    break;

    case 'L':
     lock(session, Session::State::waiting_for_lock_pull);
    break;

    case 'U':
     net::async_read
     (
      session->socket,
      net::buffer(session->buffer, 16),
      std::bind
      (
       &Server::push_handler,
       this,
       session,
       std::placeholders::_1,
       std::placeholders::_2
      )
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

    case 'i':
     write_buffer_and_next_command(session, 1);
     write_status();
    break;

    case 'Q':
     if (session->state == Session::State::locking)
      unlock(*session);
    break;

    default:
     session->write_id(std::cerr) << "unexpected command\n";
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
   std::bind
   (
    &Server::read_command_handler,
    this,
    session,
    std::placeholders::_1,
    std::placeholders::_2
   )
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
   std::bind
   (
    &Server::write_buffer_and_next_command_handler,
    this,
    session,
    std::placeholders::_1,
    std::placeholders::_2
   )
  );
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
    const int64_t client_version = from_network(session->buffer + 5);

    session->write_id(std::cerr) << "client_version = ";
    std::cerr << client_version << '\n';

    if (client_version < 3)
     to_network(0, session->buffer + 5);
    else
    {
     const int64_t server_version = 3;
     to_network(server_version, session->buffer + 5);
    }

    to_network(session->id, session->buffer + 5 + 8);

    write_buffer_and_next_command(session, 5 + 8 + 8);
   }
   else
    session->write_id(std::cerr) << "bad handshake\n";
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
    std::bind
    (
     &Server::handshake_handler,
     this,
     session,
     std::placeholders::_1,
     std::placeholders::_2
    )
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
   std::bind
   (
    &Server::handle_accept,
    this,
    std::placeholders::_1,
    std::placeholders::_2
   )
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
   std::bind
   (
    &Server::handle_interrupt_timer,
    this,
    std::placeholders::_1
   )
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::handle_interrupt_timer(std::error_code error)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!error)
  {
   if (interrupted)
   {
    std::cerr << "Interruption detected\n";
    io_context.stop();
   }

   start_interrupt_timer();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Server
 ////////////////////////////////////////////////////////////////////////////
 (
  joedb::Writable_Journal &journal,
  net::io_context &io_context,
  uint16_t port,
  uint32_t lock_timeout_seconds
 ):
  journal(journal),
  io_context(io_context),
  acceptor(io_context, net::ip::tcp::endpoint(net::ip::tcp::v4(), port)),
  port(acceptor.local_endpoint().port()),
  interrupt_timer(io_context),
  session_count(0),
  session_id(0),
  lock_timeout_seconds(lock_timeout_seconds),
  lock_timeout_timer(io_context),
  locked(false)
 {
  write_status();

  std::signal(SIGINT, signal_handler);

  start_interrupt_timer();
  start_accept();
 }
}
