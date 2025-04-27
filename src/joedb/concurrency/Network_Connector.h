#ifndef joedb_Network_Connector_declared
#define joedb_Network_Connector_declared

#include "joedb/concurrency/Network_Channel.h"
#include "joedb/concurrency/Connector.h"

namespace joedb
{
 /// @ingroup concurrency
 class Network_Connector: public Connector
 {
  private:
   const std::string host;
   const std::string service;

  public:
   Network_Connector(std::string host, std::string service):
    host(std::move(host)),
    service(std::move(service))
   {
   }

   std::unique_ptr<Channel> new_channel() const override
   {
    return std::make_unique<Network_Channel>(host, service);
   }
 };
}

#endif
