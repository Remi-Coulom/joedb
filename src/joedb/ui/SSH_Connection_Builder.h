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
   bool has_sharing_option() const final {return true;}
   int get_min_parameters() const final {return 3;}
   int get_max_parameters() const final {return 5;}
   const char *get_name() const final {return "ssh";}
   const char *get_parameters_description() const final
   {
    return "<user> <host> <endpoint_path> [<ssh_port> [<ssh_log_level>]]";
   }

   Connection &build(int argc, char **argv, Buffered_File *file) final
   {
    const char * const user = argv[0];
    const char * const host = argv[1];
    const char * const remote_path = argv[2];
    const unsigned ssh_port = argc > 3 ? std::atoi(argv[3]) : 22;
    const int ssh_log_level = argc > 4 ? std::atoi(argv[4]) : 0;

    connector = std::make_unique<ssh::Connector>
    (
     user,
     host,
     ssh_port,
     ssh_log_level,
     nullptr,
     nullptr,
     remote_path
    );

    if (file)
     connection = std::make_unique<Robust_Connection>(*connector, &std::cerr);
    else
     connection = std::make_unique<Server_File>(*connector, &std::cerr);

    return *connection;
   }
 };
}

#endif
