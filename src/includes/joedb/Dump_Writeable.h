#ifndef joedb_Dump_Writeable_declared
#define joedb_Dump_Writeable_declared

#include "Schema_Writeable.h"

#include <iosfwd>

namespace joedb
{
 class Dump_Writeable: public Schema_Writeable
 {
  protected:
   Database db;
   std::ostream &out;

   static std::string get_local_time(int64_t timestamp);

  public:
   Dump_Writeable(std::ostream &out): Schema_Writeable(db), out(out) {}
 };
}

#endif
