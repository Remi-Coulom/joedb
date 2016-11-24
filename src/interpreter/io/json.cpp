#include "json.h"
#include "joedb/Database.h"
#include "type_io.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
void joedb::write_json(std::ostream &out, const Database &db)
/////////////////////////////////////////////////////////////////////////////
{
 out << "{\n";

 bool first_table = true;
 for (auto table: db.get_tables())
 {
  if (first_table)
   first_table = false;
  else
   out << ",\n";

  out << ' ' << table.second.get_name() << ":\n";
  out << " {\n";

  const auto &fields = table.second.get_fields();
  const auto &freedom = table.second.get_freedom();

  bool first_field = true;
  for (const auto &field: fields)
  {
   if (first_field)
    first_field = false;
   else
    out << ",\n";

   out << "  " << field.second.get_name() << ": [";

   bool first_value = true;
   for (size_t i = 0; i < freedom.size(); i++)
   {
    if (first_value)
     first_value = false;
    else
     out << ',';

    const record_id_t record_id = i + 1;


    switch(field.second.get_type().get_type_id())
    {
     case Type::type_id_t::null:
     break;

     case Type::type_id_t::reference:
      out << int64_t(table.second.get_reference(record_id, field.first)) - 1;
     break;

     #define TYPE_MACRO(type, return_type, type_id, R, W)\
     case Type::type_id_t::type_id:\
      joedb::write_##type_id(out,\
       table.second.get_##type_id(record_id, field.first));\
     break;
     #define TYPE_MACRO_NO_REFERENCE
     #include "joedb/TYPE_MACRO.h"
     #undef TYPE_MACRO_NO_REFERENCE
     #undef TYPE_MACRO
    }
   }

   out << "]";
  }

  out << "\n }";
 }

 out << "\n}\n";
}
