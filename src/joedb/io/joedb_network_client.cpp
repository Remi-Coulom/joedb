#include "joedb/io/Connection_Builder.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Network_Channel.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 class Network_Connection_Builder: public Connection_Builder
 /////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::unique_ptr<Network_Channel> channel;
   std::unique_ptr<Server_Connection> connection;

  public:
   int get_min_parameters() const override {return 2;}
   int get_max_parameters() const override {return 3;}
   const char *get_parameters_description() const override
   {
    return "<host> <port> [<local_file_name>]";
   }

   void build(int argc, const char * const *argv) override
   {
    const char * const host = argv[0];
    const char * const port = argv[1];
    const char * const file_name = argc > 2 ? argv[2] : nullptr;

    open_client_file(file_name);
    channel.reset(new Network_Channel(host, port));
    connection.reset(new Server_Connection(*client_journal, *channel, &std::cerr));
   }

   Connection &get_connection() override {return *connection;}
 };

 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  return Network_Connection_Builder().main(argc, argv);
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
