#include "joedb/concurrency/Network_Channel.h"
#include "joedb/concurrency/Robust_Connection.h"
#include "joedb/concurrency/Server_File.h"
#include "joedb/ui/Connection_Builder.h"

#include <iostream>

namespace joedb
{
 /// @ingroup ui
 class Network_Connection_Builder: public Connection_Builder
 {
  private:
   std::unique_ptr<Network_Channel> channel;
   std::unique_ptr<Server_Connection> connection;

   std::unique_ptr<Network_Connector> connector;
   std::unique_ptr<Robust_Connection> robust_connection;

  public:
   bool has_sharing_option() const final {return true;}
   int get_min_parameters() const final {return 2;}
   int get_max_parameters() const final {return 2;}
   const char *get_name() const final {return "network";}
   const char *get_parameters_description() const final
   {
    return "<host> <port>";
   }

   Connection &build(int argc, char **argv, Buffered_File *file) final
   {
    const char * const host = argv[0];
    const char * const port = argv[1];

    if (file)
    {
     connector = std::make_unique<Network_Connector>(host, port);
     robust_connection = std::make_unique<Robust_Connection>(*connector, &std::cerr);
     return *robust_connection;
    }
    else
    {
     channel = std::make_unique<Network_Channel>(host, port);
     connection = std::make_unique<Server_File>(*channel);
     connection->set_log(&std::cerr);
     return *connection;
    }
   }
 };
}
