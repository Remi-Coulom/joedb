#include "joedb/concurrency/Server.h"

#include <iostream>
#include <functional>

namespace joedb
{
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
  const std::error_code &error,
  net::ip::tcp::socket socket
 )
 {
  if (!error)
  {
   connections.emplace_back(std::move(socket));
  }

  start_accept();
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
  acceptor(io_context, net::ip::tcp::endpoint(net::ip::tcp::v4(), port))
 {
  std::cerr << "port = " << acceptor.local_endpoint().port() << '\n';
  start_accept();
 }
}
