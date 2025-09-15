#ifndef joedb_Websocket_Connector_declared
#define joedb_Websocket_Connector_declared

#include "joedb/concurrency/Websocket_Channel.h"
#include "joedb/concurrency/Connector.h"

namespace joedb
{
 /// @ingroup concurrency
 class Websocket_Connector: public Connector
 {
  private:
   const std::string host;
   const std::string port;
   const std::string path;

  public:
   Websocket_Connector
   (
    std::string host,
    std::string port,
    std::string path
   ):
    host(host),
    port(port),
    path(path)
   {
   }

   std::unique_ptr<Channel> new_channel() const override
   {
    return std::make_unique<Websocket_Channel>(host, port, path);
   }
 };
}

#endif
