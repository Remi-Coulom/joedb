#ifndef joedb_SSH_Connection_Builder
#define joedb_SSH_Connection_Builder

#include "joedb/io/Connection_Builder.h"
#include "joedb/ssh/Forward_Channel.h"
#include "joedb/concurrency/Server_File.h"

#include <iostream>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 class SSH_Connection_Builder: public Connection_Builder
 /////////////////////////////////////////////////////////////////////////////
 {
  private:
   const bool file;

   std::unique_ptr<ssh::Session> session;
   std::unique_ptr<ssh::Forward_Channel> channel;
   std::unique_ptr<Server_Connection> connection;

  public:
   SSH_Connection_Builder(bool file): file(file) {}

   bool has_sharing_option() const final {return true;}
   int get_min_parameters() const final {return 3;}
   int get_max_parameters() const final {return 5;}
   const char *get_name() const final {return file ? "ssh_file" : "ssh";}
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

    session = std::make_unique<ssh::Session>(user, host, ssh_port, ssh_log_level);
    channel = std::make_unique<ssh::Forward_Channel>(*session, "localhost", joedb_port);

    if (file)
     connection = std::make_unique<Server_File>(*channel);
    else
     connection = std::make_unique<Server_Connection>(*channel);

    connection->set_log(&std::cerr);

    return *connection;
   }
 };
}

#endif
