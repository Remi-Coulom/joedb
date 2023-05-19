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

  public:
   bool has_sharing_option() const final {return true;}
   int get_min_parameters() const override {return 3;}
   int get_max_parameters() const override {return 5;}

   const char *get_parameters_description() const override
   {
    return "<user> <host> <joedb_port> [<ssh_port> [<ssh_log_level>]]";
   }

   std::unique_ptr<Connection> build
   (
    Writable_Journal &client_journal,
    int argc,
    const char * const *argv
   ) final
   {
    const char * const user = argv[0];
    const char * const host = argv[1];
    const uint16_t joedb_port = uint16_t(std::atoi(argv[2]));
    const int ssh_port = argc > 3 ? std::atoi(argv[3]) : 22;
    const int ssh_log_level = argc > 4 ? std::atoi(argv[4]) : 0;

    session.reset
    (
     new ssh::Session(user, host, ssh_port, ssh_log_level)
    );

    channel.reset
    (
     new ssh::Forward_Channel(*session, "localhost", joedb_port)
    );

    return std::unique_ptr<Connection>
    (
     new Server_Connection(client_journal, *channel, &std::cerr)
    );
   }
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
