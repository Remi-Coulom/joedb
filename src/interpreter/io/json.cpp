#include "json.h"
#include "joedb/Database.h"
#include "type_io.h"
#include "base64.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
void joedb::write_json(std::ostream &out, const Readable &db, bool base64)
/////////////////////////////////////////////////////////////////////////////
{
 //
 // First, create reference translations
 //
 std::map<table_id_t, std::vector<int64_t>> reference_translation;
 for (auto table: db.get_tables())
 {
  std::vector<int64_t> &v = reference_translation[table.first];
  const auto &freedom = table.second.get_freedom();
  v.resize(freedom.size() + 1);
  v[0] = -1;
  int64_t position = 0;
  for (size_t i = 0; i < freedom.size(); i++)
  {
   if (freedom.is_free(i + 2))
    v[i + 1] = -1;
   else
    v[i + 1] = position++;
  }
 }

 //
 // Write output
 //
 out << "{\n";

 bool first_table = true;
 for (auto table: db.get_tables())
 {
  if (first_table)
   first_table = false;
  else
   out << ",\n";

  out << ' ';
  write_string(out, table.second.get_name());
  out << ":\n {\n";

  const auto &fields = table.second.get_fields();
  const auto &freedom = table.second.get_freedom();

  bool first_field = true;
  for (const auto &field: fields)
  {
   if (first_field)
    first_field = false;
   else
    out << ",\n";

   out << "  ";
   write_string(out, field.second.get_name());
   out << ": [";

   bool first_value = true;
   for (size_t i = 0; i < freedom.size(); i++)
    if (!freedom.is_free(i + 2))
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
       out << reference_translation[field.second.get_type().get_table_id()]
               [table.second.get_reference(record_id, field.first)];
      break;

      case Type::type_id_t::string:
       if (base64)
       {
        out << '"';
        out << base64_encode(table.second.get_string(record_id, field.first));
        out << '"';
       }
       else
        joedb::write_string(out,
                            table.second.get_string(record_id, field.first));
      break;

      #define TYPE_MACRO(type, return_type, type_id, R, W)\
      case Type::type_id_t::type_id:\
       joedb::write_##type_id(out,\
        table.second.get_##type_id(record_id, field.first));\
      break;
      #define TYPE_MACRO_NO_REFERENCE
      #define TYPE_MACRO_NO_STRING
      #include "joedb/TYPE_MACRO.h"
      #undef TYPE_MACRO_NO_STRING
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
