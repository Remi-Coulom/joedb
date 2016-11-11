#include "Dump_Listener.h"
#include <ctime>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
std::string Dump_Listener::get_table_name(table_id_t table_id)
/////////////////////////////////////////////////////////////////////////////
{
 auto it = db.get_tables().find(table_id);
 if (it != db.get_tables().end())
  return it->second.get_name();
 else
  return "";
}

/////////////////////////////////////////////////////////////////////////////
std::string Dump_Listener::get_field_name
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 field_id_t field_id
)
{
 auto table = db.get_tables().find(table_id);
 if (table == db.get_tables().end())
  return "";

 auto field = table->second.get_fields().find(field_id);
 if (field == table->second.get_fields().end())
  return "";

 return field->second.get_name();
}

/////////////////////////////////////////////////////////////////////////////
Type Dump_Listener::get_field_type(table_id_t table_id, field_id_t field_id)
/////////////////////////////////////////////////////////////////////////////
{
 auto table = db.get_tables().find(table_id);
 if (table == db.get_tables().end())
  return Type();

 auto field = table->second.get_fields().find(field_id);
 if (field == table->second.get_fields().end())
  return Type();

 return field->second.get_type();
}

/////////////////////////////////////////////////////////////////////////////
std::string Dump_Listener::get_local_time(int64_t timestamp)
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
