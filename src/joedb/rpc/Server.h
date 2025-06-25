#ifndef joedb_rpc_Server_declared
#define joedb_rpc_Server_declared

#include "joedb/rpc/Procedure.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/signal_set.hpp>

#include <vector>
#include <functional>

namespace joedb::rpc
{
 /// RPC Server
 ///
 /// @ingroup RPC
 class Server
 {
  private:
   const std::chrono::time_point<std::chrono::steady_clock> start_time;
   boost::asio::io_context &io_context;
   const std::string endpoint_path;
   boost::asio::local::stream_protocol::endpoint endpoint;
   boost::asio::local::stream_protocol::acceptor acceptor;
   bool stopped;
   boost::asio::signal_set interrupt_signals;

   const std::vector<std::reference_wrapper<Procedure>> &procedures;

  public:
   Server
   (
    const std::vector<std::reference_wrapper<Procedure>> &procedures,
    boost::asio::io_context &io_context,
    std::string endpoint_path
   );
 };
}

#endif
