#ifndef joedb_Local_Connection_Builder_declared
#define joedb_Local_Connection_Builder_declared

#include "joedb/ui/Server_Connection_Builder.h"
#include "joedb/concurrency/Local_Connector.h"

namespace joedb
{
 /// @ingroup ui
 class Local_Connection_Builder: public Server_Connection_Builder
 {
  public:
   const char *get_name() const override {return "local";}

   std::string get_connection_parameters() const override
   {
    return "<path>";
   }

   void build_connector(Arguments &arguments) override
   {
    const std::string_view endpoint_path = arguments.get_next();

    if (arguments.missing())
     return;

    connector = std::make_unique<Local_Connector>(endpoint_path.data());
   }
 };
}

#endif
