#ifndef joedb_Websocket_Connection_Builder_declared
#define joedb_Websocket_Connection_Builder_declared

#include "joedb/ui/Server_Connection_Builder.h"
#include "joedb/concurrency/Websocket_Connector.h"

namespace joedb
{
 /// @ingroup ui
 class Websocket_Connection_Builder: public Server_Connection_Builder
 {
  public:
   const char *get_name() const override {return "websocket";}
   std::string get_connection_parameters() const override
   {
    return "<host> <port> <path>";
   }

   void build_connector(Arguments &arguments) override
   {
    const std::string_view host = arguments.get_next();
    const std::string_view port = arguments.get_next();
    const std::string_view path = arguments.get_next();

    if (arguments.missing())
     return;

    connector = std::make_unique<Websocket_Connector>
    (
     std::string(host),
     std::string(port),
     std::string(path)
    );
   }
 };
}

#endif
