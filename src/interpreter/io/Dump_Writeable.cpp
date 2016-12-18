#include "Dump_Writeable.h"
#include <ctime>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
std::string Dump_Writeable::get_table_name(table_id_t table_id)
/////////////////////////////////////////////////////////////////////////////
{
 for (auto table: db.get_tables())
  if (table.second == table_id)
   return table.first;

 return "__unknown_table__";
}

/////////////////////////////////////////////////////////////////////////////
std::string Dump_Writeable::get_field_name
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 field_id_t field_id
)
{
 for (auto field: db.get_table_fields(table_id))
  if (field.second == field_id)
   return field.first;
 return "__unknown_field__";
}

/////////////////////////////////////////////////////////////////////////////
std::string Dump_Writeable::get_local_time(int64_t timestamp)
/////////////////////////////////////////////////////////////////////////////
{
 const size_t buffer_size = 20;
 char buffer[buffer_size];
 time_t stamp = time_t(timestamp);

 std::strftime
 (
  buffer,
  buffer_size,
  "%Y-%m-%d %H:%M:%S",
  std::localtime(&stamp)
 );

 return buffer;
}
