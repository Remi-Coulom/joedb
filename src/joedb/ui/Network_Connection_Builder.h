#include "joedb/concurrency/Network_Channel.h"
#include "joedb/concurrency/Server_File.h"
#include "joedb/ui/Connection_Builder.h"

#include <iostream>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 class Network_Connection_Builder: public Connection_Builder
 /////////////////////////////////////////////////////////////////////////////
 {
  private:
   const bool file;

   std::unique_ptr<Network_Channel> channel;
   std::unique_ptr<Server_Connection> connection;

  public:
   Network_Connection_Builder(bool file): file(file) {}

   bool has_sharing_option() const final {return true;}
   int get_min_parameters() const final {return 2;}
   int get_max_parameters() const final {return 2;}
   const char *get_name() const final {return file ? "network_file" : "network";}
   const char *get_parameters_description() const final
   {
    return "<host> <port>";
   }

   Pullonly_Connection &build(int argc, char **argv) final
   {
    const char * const host = argv[0];
    const char * const port = argv[1];

    channel = std::make_unique<Network_Channel>(host, port);

    if (file)
     connection = std::make_unique<Server_File>(*channel);
    else
     connection = std::make_unique<Server_Connection>(*channel);

    connection->set_log(&std::cerr);

    return *connection;
   }
 };
}
