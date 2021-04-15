#include "joedb/concurrency/Server.h"

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

 ///////////////////////////////////////////////////////////////////////////
 void Server::lock_dequeue()
 ///////////////////////////////////////////////////////////////////////////
 {
  if (!locked && !lock_queue.empty())
  {
   std::cerr << "Locking\n";
   locked = true;
   Connection connection = lock_queue.front();
   lock_queue.pop();
   write_buffer(connection, 1);
  }
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::read_handler
 ///////////////////////////////////////////////////////////////////////////
 (
  Connection connection,
  const std::error_code &error,
  size_t bytes_transferred
 )
 {
  if (!error)
  {
   std::cerr << "Received command: " << connection->buffer[0] << '\n';

   switch (connection->buffer[0])
   {
    case 'l':
     if (!connection->locking)
     {
      connection->locking = true;
      lock_queue.push(connection);
      lock_dequeue();
      return;
     }
    break;

    case 'u':
     if (connection->locking)
     {
      std::cerr << "Unlocking\n";
      connection->locking = false;
      locked = false;
      write_buffer(connection, 1);
      lock_dequeue();
     }
    break;
   }

   read_some(connection);
  }

  // TODO: error management
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::read_some(Connection connection)
 ///////////////////////////////////////////////////////////////////////////
 {
  connection->socket.async_read_some
  (
   net::buffer(connection->buffer, connection->buffer_size),
   std::bind
   (
    &Server::read_handler,
    this,
    connection,
    std::placeholders::_1,
    std::placeholders::_2
   )
  );
 }

 ///////////////////////////////////////////////////////////////////////////
 void Server::write_handler
 ///////////////////////////////////////////////////////////////////////////
 (
  Connection connection,
  const std::error_code &error,
  size_t bytes_transferred
 )
 {
  if (!error)
  {
   std::cerr << "Successfully wrote data\n";
   read_some(connection);
  }

  // TODO: error management
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::write_buffer(Connection connection, size_t size)
 ////////////////////////////////////////////////////////////////////////////
 {
  net::async_write
  (
   connection->socket,
   net::buffer(connection->buffer, size),
   std::bind
   (
    &Server::write_handler,
    this,
    connection,
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
   connections.emplace_back(std::move(socket));
   std::cerr << "Created a new connection\n";

   Connection connection = std::prev(connections.end());

   connection->buffer[0] = 'j';
   connection->buffer[1] = 'o';
   connection->buffer[2] = 'e';
   connection->buffer[3] = 'd';
   connection->buffer[4] = 'b';

   write_buffer(connection, 5);
  }

  // TODO: handle errors

  start_accept();
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

  // TODO: handle errors

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
