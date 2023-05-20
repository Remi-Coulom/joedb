#include "joedb/io/Connection_Builder.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/concurrency/SSH_Server_Connection.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 class SSH_Connection_Builder: public Connection_Builder
 /////////////////////////////////////////////////////////////////////////////
 {
  public:
   bool has_sharing_option() const final {return true;}
   int get_min_parameters() const override {return 3;}
   int get_max_parameters() const override {return 5;}

   const char *get_parameters_description() const override
   {
    return "<user> <host> <joedb_port> [<ssh_port> [<ssh_log_level>]]";
   }

   std::unique_ptr<Connection> build(int argc, char **argv) final
   {
    const char * const user = argv[0];
    const char * const host = argv[1];
    const uint16_t joedb_port = uint16_t(std::atoi(argv[2]));
    const unsigned ssh_port = argc > 3 ? std::atoi(argv[3]) : 22;
    const int ssh_log_level = argc > 4 ? std::atoi(argv[4]) : 0;

    return std::unique_ptr<Connection>
    (
     new SSH_Server_Connection
     (
      user,
      host,
      joedb_port,
      ssh_port,
      ssh_log_level,
      &std::cerr
     )
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
