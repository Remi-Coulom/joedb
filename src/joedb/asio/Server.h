#ifndef joedb_asio_Server
#define joedb_asio_Server

#include <boost/asio/io_context.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/signal_set.hpp>

namespace joedb::asio
{
 /// Superclass for joedb servers
 ///
 /// @ingroup asio
 class Server
 { 
  protected:
   boost::asio::io_context &io_context;
   const std::string endpoint_path;
   boost::asio::local::stream_protocol::endpoint endpoint;
   boost::asio::local::stream_protocol::acceptor acceptor;
   bool stopped;
   boost::asio::signal_set interrupt_signals;

  public:
   Server(boost::asio::io_context &io_context, std::string endpoint_path);
   ~Server();
 };
}

#endif
