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
 std::map<Table_Id, std::vector<int64_t>> reference_translation;
 std::map<Table_Id, int64_t> table_size;
 for (auto table: db.get_tables())
 {
  const Table_Id table_id = table.first;
  const Record_Id last_record_id = db.get_last_record_id(table_id);
  std::vector<int64_t> &v = reference_translation[table_id];
  v.resize(db.get_last_record_id(table_id) + 1);
  v[0] = -1;
  int64_t position = 0;
  for (Record_Id record_id = 1; record_id <= last_record_id; record_id++)
  {
   if (!db.is_used(table_id, record_id))
    v[record_id] = -1;
   else
    v[record_id] = position++;
  }
  table_size[table_id] = position;
 }

 //
 // Write output
 //
 out << "{\n";

 bool first_table = true;
 for (auto table: db.get_tables())
 {
  const Table_Id table_id = table.first;
  const Record_Id last_record_id = db.get_last_record_id(table_id);

  if (first_table)
   first_table = false;
  else
   out << ",\n";

  out << " \"" << table.second << "\":\n {\n";

  out << "  \"__size\": " << table_size[table_id];

  for (const auto &field: db.get_fields(table_id))
  {
   const Field_Id field_id = field.first;
   const Type &type = db.get_field_type(table_id, field_id);
   out << ",\n";

   out << "  \"" << field.second << "\": [";

   bool first_value = true;
   for (Record_Id record_id = 1; record_id <= last_record_id; record_id++)
    if (db.is_used(table_id, record_id))
    {
     if (first_value)
      first_value = false;
     else
      out << ',';

     switch(type.get_type_id())
     {
      case Type::Type_Id::null:
      break;

      case Type::Type_Id::reference:
      {
       Record_Id i = db.get_reference(table_id, record_id, field_id);
       const std::vector<int64_t> &v = reference_translation[type.get_table_id()];
       if (i >= v.size())
        i = 0;
       out << v[i];
      }
      break;

      case Type::Type_Id::string:
       if (base64)
       {
        out << '"';
        out << base64_encode(db.get_string(table_id, record_id, field_id));
        out << '"';
       }
       else
        joedb::write_string(out, db.get_string(table_id, record_id, field_id));
      break;

      #define TYPE_MACRO(type, return_type, type_id, R, W)\
      case Type::Type_Id::type_id:\
       joedb::write_##type_id(out,\
        db.get_##type_id(table_id, record_id, field_id));\
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
