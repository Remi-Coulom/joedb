#ifndef joedb_Dummy_Connection_Builder_declared
#define joedb_Dummy_Connection_Builder_declared

#include "joedb/io/Connection_Builder.h"
#include "joedb/concurrency/Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Dummy_Connection_Builder: public Connection_Builder
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Full_Connection connection;

  public:
   const char *get_name() const final
   {
    return "dummy";
   }

   Connection &build(int argc, char **argv) final
   {
    return connection;
   }
 };
}

#endif
