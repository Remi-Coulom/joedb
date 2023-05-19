#include "joedb/io/Connection_Builder.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/concurrency/Server_Connection.h"
#include "joedb/ssh/Forward_Channel.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 class SSH_Connection_Builder: public Connection_Builder
 /////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::unique_ptr<ssh::Session> session;
   std::unique_ptr<ssh::Forward_Channel> channel;
   std::unique_ptr<Server_Connection> connection;

  public:
   int get_min_parameters() const override {return 3;}
   int get_max_parameters() const override {return 6;}

   const char *get_parameters_description() const override
   {
    return "<user> <host> <joedb_port> [<local_file_name> [<ssh_port> [<ssh_log_level>]]]";
   }

   void build(int argc, const char * const *argv) override
   {
    const char * const user = argv[0];
    const char * const host = argv[1];
    const uint16_t joedb_port = uint16_t(std::atoi(argv[2]));
    const char * const local_file_name = argc > 3 ? argv[3] : nullptr;
    const int ssh_port = argc > 4 ? std::atoi(argv[4]) : 22;
    const int ssh_log_level = argc > 5 ? std::atoi(argv[5]) : 0;

    open_client_file(local_file_name);
    session.reset(new ssh::Session(user, host, ssh_port, ssh_log_level));
    channel.reset(new ssh::Forward_Channel(*session, "localhost", joedb_port));
    connection.reset(new Server_Connection(*client_journal, *channel, &std::cerr));
   }

   Connection &get_connection() override {return *connection;}
 };

 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  return SSH_Connection_Builder().main(argc, argv);
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
