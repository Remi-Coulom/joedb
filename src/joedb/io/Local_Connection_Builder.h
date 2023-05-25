#ifndef joedb_Local_Connection_Builder_declared
#define joedb_Local_Connection_Builder_declared

#include "joedb/io/Connection_Builder.h"
#include "joedb/concurrency/Local_Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Local_Connection_Builder: public Connection_Builder
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   const char *get_name() const final
   {
    return "local";
   }

   bool get_default_sharing() const final
   {
    return true;
   }

   std::unique_ptr<Connection> build(int argc, char **argv) final
   {
    return std::unique_ptr<Connection>(new Local_Connection());
   }
 };
}

#endif
