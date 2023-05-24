#include "joedb/io/Connection_Builder.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/client_main.h"
#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Network_Channel.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 class Network_Channel_Connection:
 /////////////////////////////////////////////////////////////////////////////
  public Network_Channel,
  public Server_Connection
 {
  public:
   Network_Channel_Connection(const char *host, const char *port):
    Network_Channel(host, port),
    Server_Connection(*this, &std::cerr)
   {
   }
 };

 /////////////////////////////////////////////////////////////////////////////
 class Network_Connection_Builder: public Connection_Builder
 /////////////////////////////////////////////////////////////////////////////
 {
  public:
   bool has_sharing_option() const final {return true;}
   int get_min_parameters() const final {return 2;}
   int get_max_parameters() const final {return 2;}
   const char *get_parameters_description() const final
   {
    return "<host> <port>";
   }

   std::unique_ptr<Connection> build(int argc, char **argv) final
   {
    const char * const host = argv[0];
    const char * const port = argv[1];

    return std::unique_ptr<Connection>
    (
     new Network_Channel_Connection(host, port)
    );
   }
 };

 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  return client_main(argc, argv, Network_Connection_Builder());
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
