#ifndef joedb_Dummy_Connection_Builder_declared
#define joedb_Dummy_Connection_Builder_declared

#include "joedb/ui/Connection_Builder.h"
#include "joedb/concurrency/Connection.h"

namespace joedb::ui
{
 ////////////////////////////////////////////////////////////////////////////
 class Dummy_Connection_Builder: public Connection_Builder
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Connection connection;

  public:
   const char *get_name() const final
   {
    return "dummy";
   }

   Pullonly_Connection &build(int argc, char **argv, Buffered_File *file) final
   {
    return connection;
   }
 };
}

#endif
