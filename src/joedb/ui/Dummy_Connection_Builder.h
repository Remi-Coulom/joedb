#ifndef joedb_Dummy_Connection_Builder_declared
#define joedb_Dummy_Connection_Builder_declared

#include "joedb/ui/Connection_Builder.h"
#include "joedb/concurrency/Connection.h"

namespace joedb
{
 /// @ingroup ui
 class Dummy_Connection_Builder: public Connection_Builder
 {
  public:
   const char *get_name() const override
   {
    return "dummy";
   }

   Connection *build(Arguments &arguments, Buffered_File *file) override
   {
    return &Connection::dummy;
   }
 };
}

#endif
