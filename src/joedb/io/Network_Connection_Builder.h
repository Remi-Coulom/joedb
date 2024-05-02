#include "joedb/concurrency/Network_Channel.h"
#include "joedb/concurrency/Server_Connection.h"
#include "joedb/io/Connection_Builder.h"

#include <iostream>

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
  private:
   std::unique_ptr<Network_Channel_Connection> connection;

  public:
   bool has_sharing_option() const final {return true;}
   int get_min_parameters() const final {return 2;}
   int get_max_parameters() const final {return 2;}
   const char *get_name() const final {return "network";}
   const char *get_parameters_description() const final
   {
    return "<host> <port>";
   }

   Connection &build(int argc, char **argv) final
   {
    const char * const host = argv[0];
    const char * const port = argv[1];

    connection.reset(new Network_Channel_Connection(host, port));

    return *connection;
   }
 };
}
