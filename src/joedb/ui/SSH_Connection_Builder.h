#ifndef joedb_SSH_Connection_Builder
#define joedb_SSH_Connection_Builder

#include "joedb/ui/Connection_Builder.h"
#include "joedb/ssh/Connector.h"
#include "joedb/concurrency/Server_File.h"

#include <iostream>

namespace joedb
{
 /// @ingroup ui
 class SSH_Connection_Builder: public Connection_Builder
 {
  private:
   std::unique_ptr<ssh::Connector> connector;
   std::unique_ptr<Connection> connection;

  public:
   bool has_sharing_option() const override {return true;}
   const char *get_name() const override {return "ssh";}
   const char *get_parameters_description() const override
   {
    return "<user> <host> <endpoint_path> [<ssh_port> [<ssh_log_level>]]";
   }

   Connection *build(Arguments &arguments, Buffered_File *file) override
   {
    const std::string_view user = arguments.get_next();
    const std::string_view host = arguments.get_next();
    const std::string_view remote_path = arguments.get_next();

    if (arguments.missing())
     return nullptr;

    const std::string_view port_string = arguments.get_next();
    const std::string_view log_level_string = arguments.get_next();

    unsigned ssh_port = 22;
    if (port_string.data())
     ssh_port = std::atoi(port_string.data());

    int ssh_log_level =  0;
    if (log_level_string.data())
     ssh_log_level = std::atoi(log_level_string.data());

    connector = std::make_unique<ssh::Connector>
    (
     user.data(),
     host.data(),
     ssh_port,
     ssh_log_level,
     nullptr,
     nullptr,
     remote_path.data()
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
