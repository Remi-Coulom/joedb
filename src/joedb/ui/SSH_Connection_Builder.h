#ifndef joedb_SSH_Connection_Builder
#define joedb_SSH_Connection_Builder

#include "joedb/ui/Server_Connection_Builder.h"
#include "joedb/ssh/Connector.h"

namespace joedb
{
 /// @ingroup ui
 class SSH_Connection_Builder: public Server_Connection_Builder
 {
  public:
   const char *get_name() const override {return "ssh";}
   std::string get_connection_parameters() const override
   {
    return "[--port p] [--verbosity v] <user> <host> <path>";
   }

   void build_connector(Arguments &arguments) override
   {
    const auto port = arguments.next_option<unsigned>("port", "p", 22);
    const auto verbosity = arguments.next_option<int>("verbosity", "v", 0);
    const std::string_view user = arguments.get_next();
    const std::string_view host = arguments.get_next();
    const std::string_view path = arguments.get_next();

    if (arguments.missing())
     return;

    connector = std::make_unique<ssh::Connector>
    (
     user.data(),
     host.data(),
     port,
     verbosity,
     nullptr,
     nullptr,
     path.data()
    );
   }
 };
}

#endif
