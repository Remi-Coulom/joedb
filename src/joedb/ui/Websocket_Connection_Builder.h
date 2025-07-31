#ifndef joedb_Websocket_Connection_Builder_declared
#define joedb_Websocket_Connection_Builder_declared

#include "joedb/concurrency/Websocket_Connector.h"
#include "joedb/concurrency/Robust_Connection.h"
#include "joedb/concurrency/Server_File.h"
#include "joedb/ui/Connection_Builder.h"

#include <iostream>

namespace joedb
{
 /// @ingroup ui
 class Websocket_Connection_Builder: public Connection_Builder
 {
  private:
   std::unique_ptr<Websocket_Connector> connector;
   std::unique_ptr<Robust_Connection> connection;

  public:
   bool has_sharing_option() const override {return true;}
   const char *get_name() const override {return "websocket";}
   const char *get_parameters_description() const override
   {
    return "<host> <port> <path>";
   }

   Connection *build(Arguments &arguments, Buffered_File *file) override
   {
    const std::string_view host = arguments.get_next();
    const std::string_view port = arguments.get_next();
    const std::string_view path = arguments.get_next();

    if (arguments.missing())
     return nullptr;

    connector = std::make_unique<Websocket_Connector>
    (
     std::string(host),
     std::string(port),
     std::string(path)
    );

    if (file)
     connection = std::make_unique<Robust_Connection>(*connector, &std::cerr);
    else
     connection = std::make_unique<Server_File>(*connector, &std::cerr);

    return connection.get();
   }
 };
}

#endif
