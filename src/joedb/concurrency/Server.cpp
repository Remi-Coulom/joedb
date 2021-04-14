#include "joedb/concurrency/Server.h"

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
  }

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
  timer(io_context)
 {
  std::cerr << "port = " << acceptor.local_endpoint().port() << '\n';

  std::signal(SIGINT, signal_handler);

  start_interrupt_timer();
  start_accept();
 }
}
