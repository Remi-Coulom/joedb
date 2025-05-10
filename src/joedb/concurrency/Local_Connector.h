#ifndef joedb_Local_Connector_declared
#define joedb_Local_Connector_declared

#include "joedb/concurrency/Local_Channel.h"
#include "joedb/concurrency/Connector.h"

namespace joedb
{
 /// @ingroup concurrency
 class Local_Connector: public Connector
 {
  private:
   const std::string endpoint_path;

  public:
   Local_Connector(std::string endpoint_path):
    endpoint_path(std::move(endpoint_path))
   {
   }

   Local_Connector(std::string_view endpoint_path):
    endpoint_path(endpoint_path)
   {
   }

   Local_Connector(const char *endpoint_path):
    Local_Connector(std::string_view(endpoint_path))
   {
   }

   std::unique_ptr<Channel> new_channel() const override
   {
    return std::make_unique<Local_Channel>(endpoint_path);
   }
 };
}

#endif
