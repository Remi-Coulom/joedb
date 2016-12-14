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

   std::string get_table_name(table_id_t table_id);
   std::string get_field_name(table_id_t table_id, field_id_t field_id);
   static std::string get_local_time(int64_t timestamp);

  public:
   Dump_Writeable(std::ostream &out): Schema_Writeable(db), out(out) {}
 };
}

#endif
