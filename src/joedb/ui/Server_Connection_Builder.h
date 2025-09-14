#ifndef joedb_Server_Connection_Builder_declared
#define joedb_Server_Connection_Builder_declared

#include "joedb/ui/Connection_Builder.h"
#include "joedb/concurrency/Server_File.h"

#include <iostream>

namespace joedb
{
 /// @ingroup ui
 class Server_Connection_Builder: public Connection_Builder
 {
  protected:
   std::unique_ptr<Connector> connector;
   std::unique_ptr<Connection> connection;

  public:
   virtual void build_connector(Arguments &arguments) = 0;
   virtual std::string get_connection_parameters() const = 0;

   std::string get_parameters_description() const override final
   {
    return "[--keep_alive <seconds>] " + get_connection_parameters();
   }

   Connection *build(Arguments &arguments, Abstract_File *file) override
   {
    const float keep_alive_interval = arguments.next_option<float>
    (
     "keep_alive",
     "<seconds>",
     0.0f
    );

    build_connector(arguments);

    if (!connector)
     return nullptr;

    connector->set_keep_alive_interval
    (
     std::chrono::milliseconds(int(keep_alive_interval * 1000.0f))
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
