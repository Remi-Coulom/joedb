#ifndef joedb_Dump_Writable_declared
#define joedb_Dump_Writable_declared

#include "joedb/io/Schema_Writable.h"
#include "joedb/interpreter/Database.h"

#include <iosfwd>

namespace joedb
{
 class Dump_Writable: public Schema_Writable
 {
  protected:
   Database db;
   std::ostream &out;

  public:
   Dump_Writable(std::ostream &out): Schema_Writable(db), out(out) {}

   static std::string get_local_time(int64_t timestamp);
 };
}

#endif
