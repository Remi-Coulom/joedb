#ifndef joedb_rpc_Server_declared
#define joedb_rpc_Server_declared

#include "joedb/rpc/Procedure.h"
#include "joedb/asio/Server.h"

#include <vector>
#include <functional>

namespace joedb::rpc
{
 /// RPC Server
 ///
 /// @ingroup RPC
 class Server: public joedb::asio::Server
 {
  private:
   const std::vector<std::reference_wrapper<Procedure>> &procedures;

  public:
   Server
   (
    const std::vector<std::reference_wrapper<Procedure>> &procedures,
    boost::asio::io_context &io_context,
    std::string endpoint_path
   ):
    joedb::asio::Server(io_context, endpoint_path),
    procedures(procedures)
   {
   }

   void start();
 };
}

#endif
