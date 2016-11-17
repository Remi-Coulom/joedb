#ifndef joedb_Dump_Listener_declared
#define joedb_Dump_Listener_declared

#include "Schema_Listener.h"

#include <iosfwd>

namespace joedb
{
 class Dump_Listener: public Schema_Listener
 {
  protected:
   Database db;
   std::ostream &out;

   std::string get_table_name(table_id_t table_id);
   std::string get_field_name(table_id_t table_id, field_id_t field_id);
   static std::string get_local_time(int64_t timestamp);

  public:
   Dump_Listener(std::ostream &out): Schema_Listener(db), out(out) {}
 };
}

#endif
