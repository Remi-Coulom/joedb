#ifndef joedb_Dump_Connection_Builder
#define joedb_Dump_Connection_Builder

#include "joedb/io/Connection_Builder.h"
#include "joedb/io/Dump_Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Dump_Connection_Builder: public Connection_Builder
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   const char *get_name() const final {return "dump";}
   std::unique_ptr<Connection> build(int argc, char **argv) final
   {
    return std::unique_ptr<Connection>(new Dump_Connection());
   }
 };
}

#endif
