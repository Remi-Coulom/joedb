#ifndef joedb_Local_Connection_Builder_declared
#define joedb_Local_Connection_Builder_declared

#include "joedb/concurrency/Local_Connector.h"
#include "joedb/concurrency/Robust_Connection.h"
#include "joedb/concurrency/Server_File.h"
#include "joedb/ui/Connection_Builder.h"

#include <iostream>

namespace joedb
{
 /// @ingroup ui
 class Local_Connection_Builder: public Connection_Builder
 {
  private:
   std::unique_ptr<Local_Connector> connector;
   std::unique_ptr<Robust_Connection> connection;

  public:
   bool has_sharing_option() const final {return true;}
   int get_min_parameters() const final {return 1;}
   int get_max_parameters() const final {return 1;}
   const char *get_name() const final {return "local";}
   const char *get_parameters_description() const final
   {
    return "<endpoint_path>";
   }

   Connection &build(int argc, char **argv, Buffered_File *file) final
   {
    const char * const endpoint_path = argv[0];

    connector = std::make_unique<Local_Connector>(endpoint_path);

    if (file)
     connection = std::make_unique<Robust_Connection>(*connector, &std::cerr);
    else
     connection = std::make_unique<Server_File>(*connector, &std::cerr);

    return *connection;
   }
 };
}

#endif
