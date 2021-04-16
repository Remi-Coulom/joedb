#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/network_integers.h"

#include <iostream>
#include <functional>
#include <csignal>

#include <experimental/buffer>

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
  locking(false)
 {
  std::cerr << "Created a new session\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Session::~Session()
 ////////////////////////////////////////////////////////////////////////////
 {
  std::cerr << "Destroyed Session\n";
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::lock_dequeue()
 ///////////////////////////////////////////////////////////////////////////
 {
  if (!locked && !lock_queue.empty())
  {
   std::cerr << "Locking\n";
   locked = true;
   write_buffer(lock_queue.front(), 1);
   lock_queue.pop();
  }
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::pull_handler
 ///////////////////////////////////////////////////////////////////////////
 (
  std::shared_ptr<Session> session,
  const std::error_code &error,
  size_t bytes_transferred
 )
 {
  if (!error)
  {
   const int64_t checkpoint = from_network(session->buffer);
   std::cerr << "Pulling from checkpoint = " << checkpoint << '\n';
   session->buffer[0] = 'p';
   to_network(checkpoint, session->buffer + 1);

   const int64_t size = journal.get_checkpoint_position() - checkpoint;
   to_network(size, session->buffer + 9);
   write_buffer(session, 17);
  }
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::read_command_handler
 ///////////////////////////////////////////////////////////////////////////
 (
  std::shared_ptr<Session> session,
  const std::error_code &error,
  size_t bytes_transferred
 )
 {
  if (!error)
  {
   std::cerr << "Received command: " << session->buffer[0] << '\n';

   switch (session->buffer[0])
   {
    case 'p':
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
    break;

    case 'l':
     if (!session->locking)
     {
      session->locking = true;
      lock_queue.push(session);
      lock_dequeue();
     }
    break;

    case 'u':
     if (session->locking)
     {
      std::cerr << "Unlocking\n";
      session->locking = false;
      locked = false;
      write_buffer(session, 1);
      lock_dequeue();
     }
    break;
   }

   read_command(session);
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
 void Server::write_handler
 ///////////////////////////////////////////////////////////////////////////
 (
  std::shared_ptr<Session> session,
  const std::error_code &error,
  size_t bytes_transferred
 )
 {
  if (!error)
  {
   std::cerr << "Successfully wrote data\n";
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::write_buffer(std::shared_ptr<Session> session, size_t size)
 ////////////////////////////////////////////////////////////////////////////
 {
  net::async_write
  (
   session->socket,
   net::buffer(session->buffer, size),
   std::bind
   (
    &Server::write_handler,
    this,
    session,
    std::placeholders::_1,
    std::placeholders::_2
   )
  );
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

   session->buffer[0] = 'j';
   session->buffer[1] = 'o';
   session->buffer[2] = 'e';
   session->buffer[3] = 'd';
   session->buffer[4] = 'b';

   write_buffer(session, 5);
   read_command(session);
  }

  start_accept();
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
  timer.expires_after(std::chrono::seconds(interrupt_check_seconds));
  timer.async_wait
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
  if (interrupted)
  {
   std::cerr << "Interruption detected\n";
   io_context.stop();
  }

  start_interrupt_timer();
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Server
 ////////////////////////////////////////////////////////////////////////////
 (
  joedb::Writable_Journal &journal,
  net::io_context &io_context,
  uint16_t port
 ):
  journal(journal),
  io_context(io_context),
  acceptor(io_context, net::ip::tcp::endpoint(net::ip::tcp::v4(), port)),
  timer(io_context),
  locked(false)
 {
  std::cerr << "port = " << acceptor.local_endpoint().port() << '\n';

  std::signal(SIGINT, signal_handler);

  start_interrupt_timer();
  start_accept();
 }
}
