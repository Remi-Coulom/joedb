#ifndef joedb_Local_Connection_Builder_declared
#define joedb_Local_Connection_Builder_declared

#include "joedb/concurrency/Local_Connector.h"
#include "joedb/concurrency/Robust_Connection.h"
#include "joedb/concurrency/Server_File.h"
#include "joedb/ui/Connection_Builder.h"

#include <iostream>

namespace joedb
{
 /// @ingroup ui
 class Local_Connection_Builder: public Connection_Builder
 {
  private:
   std::unique_ptr<Local_Connector> connector;
   std::unique_ptr<Robust_Connection> connection;

  public:
   bool has_sharing_option() const override {return true;}
   const char *get_name() const override {return "local";}
   const char *get_parameters_description() const override
   {
    return "<endpoint_path>";
   }

   Connection *build(Arguments &arguments, Buffered_File *file) override
   {
    const std::string_view endpoint_path = arguments.get_next();

    if (arguments.missing())
     return nullptr;

    connector = std::make_unique<Local_Connector>(endpoint_path.data());

    if (file)
     connection = std::make_unique<Robust_Connection>(*connector, &std::cerr);
    else
     connection = std::make_unique<Server_File>(*connector, &std::cerr);

    return connection.get();
   }
 };
}

#endif
