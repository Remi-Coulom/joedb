#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/network_integers.h"

#include <iostream>
#include <functional>
#include <csignal>

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
 Server::Session::Session(net::ip::tcp::socket && socket):
 ////////////////////////////////////////////////////////////////////////////
  socket(std::move(socket)),
  state(not_locking)
 {
  std::cerr << "Created a new session\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Session::~Session()
 ////////////////////////////////////////////////////////////////////////////
 {
  std::cerr << "Destroyed Session\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::lock_dequeue()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!locked && !lock_queue.empty())
  {
   std::cerr << "Locking\n";
   locked = true;
   std::shared_ptr<Session> session = lock_queue.front();
   lock_queue.pop();

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
 void Server::unlock(std::shared_ptr<Session> session)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (session->state == Session::State::locking)
  {
   std::cerr << "Unlocking\n";
   session->state = Session::State::not_locking;
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
   std::cerr << "Timeout!\n";
   unlock(session);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::push_transfer_handler
 ////////////////////////////////////////////////////////////////////////////
 (
  std::shared_ptr<Session> session,
  int64_t size,
  std::unique_ptr<Writable_Journal::Tail_Writer> writer,
  bool conflict,
  std::error_code error,
  size_t bytes_transferred
 )
 {
  if (!error)
  {
   std::cerr << '.';

   if (writer)
    writer->append(session->buffer, bytes_transferred);
   size -= bytes_transferred;
   push_transfer(session, size, std::move(writer), conflict);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::push_transfer
 ////////////////////////////////////////////////////////////////////////////
 (
  std::shared_ptr<Session> session,
  int64_t size,
  std::unique_ptr<Writable_Journal::Tail_Writer> writer,
  bool conflict
 )
 {
  if (size > 0)
  {
   std::cerr << '.';

   net::async_read
   (
    session->socket,
    net::buffer(session->buffer, size_t(size)),
    [this, size, session, moved_writer = std::move(writer), conflict]
    (
     std::error_code error,
     size_t bytes_transferred
    ) mutable
    {
     push_transfer_handler
     (
      session,
      size,
      std::move(moved_writer),
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
    session->buffer[0] = 'U';

   std::cerr << " done. Returning '" << session->buffer[0] << "'\n";

   write_buffer_and_next_command(session, 1);

   unlock(session);
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
    std::cerr << "Trying to push while not locking.\n";
    if (!conflict && !locked)
    {
     std::cerr << "OK, this session can take the lock.\n";
     lock(session, Session::State::locking);
    }
   }

   if (session->state == Session::State::locking)
    lock_timeout_timer.cancel();

   std::unique_ptr<Writable_Journal::Tail_Writer> writer;
   if (!conflict && size > 0)
    writer.reset(new Writable_Journal::Tail_Writer(journal));

   std::cerr << "Pushing, start = " << start << ", size = " << size << ':';

   push_transfer
   (
    session,
    size,
    std::move(writer),
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
   const int64_t checkpoint = from_network(session->buffer);
   session->buffer[0] = 'p';
   to_network(checkpoint, session->buffer + 1);

   Async_Reader reader = journal.get_tail_reader(checkpoint);
   to_network(reader.get_remaining(), session->buffer + 9);

   std::cerr << "Pulling from checkpoint = " << checkpoint;
   std::cerr << ", size = " << reader.get_remaining() << ':';

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
   net::buffer(session->buffer, 8),
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
   std::cerr << "Received command: " << session->buffer[0] << '\n';

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
      unlock(session);
     else
      session->buffer[0] = 't';
     write_buffer_and_next_command(session, 1);
    break;

    case 'i':
     write_buffer_and_next_command(session, 1);
    break;

    case 'Q':
     if (session->state == Session::State::locking)
      unlock(session);
    break;

    default:
     std::cerr << "Unexpected command\n";
    break;
   }
  }
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::read_command(std::shared_ptr<Session> session)
 ///////////////////////////////////////////////////////////////////////////
 {
  std::cerr << "Waiting for next command.\n";
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

    std::cerr << "client_version = " << client_version << '\n';
    const int64_t server_version = 1;
    to_network(server_version, session->buffer + 5);

    write_buffer_and_next_command(session, 13);
   }
   else
    std::cerr << "Bad handshake\n";
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
   std::shared_ptr<Session> session(new Session(std::move(socket)));

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
  interrupt_timer(io_context),
  lock_timeout_seconds(lock_timeout_seconds),
  lock_timeout_timer(io_context),
  locked(false)
 {
  std::cerr << "port = " << acceptor.local_endpoint().port() << '\n';

  std::signal(SIGINT, signal_handler);

  start_interrupt_timer();
  start_accept();
 }
}
