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
  private:
   Dump_Connection connection{false};

  public:
   const char *get_name() const final {return "dump";}

   Connection &build(int argc, char **argv) final
   {
    return connection;
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Tail_Connection_Builder: public Connection_Builder
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Dump_Connection connection{true};

  public:
   const char *get_name() const final {return "tail";}

   Connection &build(int argc, char **argv) final
   {
    return connection;
   }
 };
}

#endif
