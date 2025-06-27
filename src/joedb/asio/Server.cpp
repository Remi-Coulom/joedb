#include "joedb/asio/Server.h"

namespace joedb::asio
{
 Server::Server
 ( 
  boost::asio::io_context &io_context,
  std::string endpoint_path
 ):
  io_context(io_context),
  endpoint_path(std::move(endpoint_path)),
  endpoint(this->endpoint_path),
  acceptor(io_context, endpoint, false),
  stopped(true),
  interrupt_signals(io_context, SIGINT, SIGTERM)
 { 
 }

 Server::~Server()
 {
  try
  {
   std::remove(endpoint_path.c_str());
  }
  catch (...)
  {
  }
 }
}

