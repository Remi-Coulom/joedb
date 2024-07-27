#ifndef joedb_SSH_Connection_Builder
#define joedb_SSH_Connection_Builder

#include "joedb/io/Connection_Builder.h"
#include "joedb/concurrency/SSH_Server_Connection.h"

#include <iostream>
#include <optional>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 class SSH_Connection_Builder: public Connection_Builder
 /////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::optional<SSH_Server_Connection> connection;

  public:
   bool has_sharing_option() const final {return true;}
   int get_min_parameters() const final {return 3;}
   int get_max_parameters() const final {return 5;}

   const char *get_name() const final {return "ssh";}

   const char *get_parameters_description() const final
   {
    return "<user> <host> <joedb_port> [<ssh_port> [<ssh_log_level>]]";
   }

   Pullonly_Connection &build(int argc, char **argv) final
   {
    const char * const user = argv[0];
    const char * const host = argv[1];
    const uint16_t joedb_port = uint16_t(std::atoi(argv[2]));
    const unsigned ssh_port = argc > 3 ? std::atoi(argv[3]) : 22;
    const int ssh_log_level = argc > 4 ? std::atoi(argv[4]) : 0;

    connection.emplace
    (
     user,
     host,
     joedb_port,
     ssh_port,
     ssh_log_level,
     &std::cerr
    );

    return *connection;
   }
 };
}

#endif
