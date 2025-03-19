#include "joedb/concurrency/Network_Channel.h"
#include "joedb/concurrency/Server_File.h"
#include "joedb/io/Connection_Builder.h"

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
   bool has_sharing_option() const final {return true;}
   int get_min_parameters() const final {return 2;}
   int get_max_parameters() const final {return 2;}
   const char *get_name() const final {return "network";}
   const char *get_parameters_description() const final
   {
    return "<host> <port>";
   }

   Pullonly_Connection &build(int argc, char **argv) final
   {
    const char * const host = argv[0];
    const char * const port = argv[1];

    channel = std::make_unique<Network_Channel>(host, port);

    connection = std::make_unique<Server_File>(*channel);

    return *connection;
   }
 };
}
