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

  public:
   bool has_sharing_option() const final {return true;}
   int get_min_parameters() const final {return 2;}
   int get_max_parameters() const final {return 2;}
   const char *get_parameters_description() const final
   {
    return "<host> <port>";
   }

   std::unique_ptr<Connection> build
   (
    Writable_Journal &client_journal,
    int argc,
    const char * const *argv
   ) final
   {
    const char * const host = argv[0];
    const char * const port = argv[1];

    channel.reset(new Network_Channel(host, port));
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
  return Network_Connection_Builder().main(argc, argv);
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
