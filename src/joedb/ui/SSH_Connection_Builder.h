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
    return "[--port p] [--verbosity v] [--key base64] [--passphrase secret] <user> <host> <path>";
   }

   void build_connector(Arguments &arguments) override
   {
    const auto port = arguments.next_option<unsigned>("port", "p", 22);
    const auto verbosity = arguments.next_option<int>("verbosity", "v", 0);
    const auto key = arguments.next_option<beman::cstring_view>("key", "base64", "");
    const auto passphrase = arguments.next_option<beman::cstring_view>("passphrase", "secret", "");

    const beman::cstring_view user = arguments.get_next("user");
    const beman::cstring_view host = arguments.get_next("host");
    const beman::cstring_view path = arguments.get_next("path");

    if (arguments.missing())
     return;

    connector = std::make_unique<ssh::Connector>
    (
     std::string(user),
     std::string(host),
     port,
     verbosity,
     std::string(key),
     std::string(passphrase),
     path.c_str()
    );
   }
 };
}

#endif
