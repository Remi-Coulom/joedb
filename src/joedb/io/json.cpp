#include "joedb/io/json.h"
#include "joedb/io/type_io.h"
#include "joedb/io/base64.h"
#include "joedb/Readable.h"
#include "joedb/Exception.h"

#include <iostream>
#include <cmath>
#include <vector>

/////////////////////////////////////////////////////////////////////////////
int joedb::write_json
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const Readable &db,
 bool base64
)
{
 int result = JSON_Error::ok;

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
  v.resize(size_t(db.get_last_record_id(table_id)) + 1);
  v[0] = -1;
  int64_t position = 0;
  for (Record_Id record_id = Record_Id(1); record_id <= last_record_id; ++record_id)
  {
   if (!db.is_used(table_id, record_id))
    v[size_t(record_id)] = -1;
   else
    v[size_t(record_id)] = position++;
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
   for (Record_Id record_id = Record_Id(1); record_id <= last_record_id; ++record_id)
   {
    if (db.is_used(table_id, record_id))
    {
     if (first_value)
      first_value = false;
     else
      out << ", ";

     switch (type.get_type_id())
     {
      case Type::Type_Id::null:
      break;

      case Type::Type_Id::reference:
      {
       Record_Id i = db.get_reference(table_id, record_id, field_id);
       const auto it = reference_translation.find(type.get_table_id());
       if (it != reference_translation.end())
       {
        const std::vector<int64_t> &v = it->second;
        if (size_t(i) >= v.size())
         i = Record_Id(0);
        out << v[size_t(i)];
       }
       else
        out << i; // reference to a missing table
      }
      break;

      case Type::Type_Id::string:
      {
       const std::string &s = db.get_string(table_id, record_id, field_id);
       result |= write_json_string(out, s, base64);
      }
      break;

      case Type::Type_Id::blob:
      {
       const Blob blob = db.get_blob(table_id, record_id, field_id);
       out << blob.get_position();
      }
      break;

      #define TYPE_MACRO(type, return_type, type_id, R, W)\
      case Type::Type_Id::type_id:\
      {\
       const auto x = db.get_##type_id(table_id, record_id, field_id);\
       if (std::isnan(x) || std::isinf(x))\
       {\
        out << '0';\
        result |= JSON_Error::infnan;\
       }\
       else\
        joedb::write_##type_id(out, x);\
      }\
      break;
      #define TYPE_MACRO_NO_REFERENCE
      #define TYPE_MACRO_NO_STRING
      #define TYPE_MACRO_NO_BLOB
      #define TYPE_MACRO_NO_INT
      #include "joedb/TYPE_MACRO.h"

      #define TYPE_MACRO(type, return_type, type_id, R, W)\
      case Type::Type_Id::type_id:\
      {\
       const auto x = db.get_##type_id(table_id, record_id, field_id);\
       joedb::write_##type_id(out, x);\
      }\
      break;
      #define TYPE_MACRO_NO_REFERENCE
      #define TYPE_MACRO_NO_STRING
      #define TYPE_MACRO_NO_BLOB
      #define TYPE_MACRO_NO_FLOAT
      #include "joedb/TYPE_MACRO.h"
     }
    }
   }

   out << "]";
  }

  out << "\n }";
 }

 out << "\n}\n";

 return result;
}

/////////////////////////////////////////////////////////////////////////////
int joedb::write_json_string
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const std::string &s,
 bool base64
)
{
 if (base64)
 {
  out << '"';
  out << base64_encode(s);
  out << '"';
 }
 else
 {
  try
  {
   joedb::write_string(out, s, true);
  }
  catch (const joedb::Exception &)
  {
   out << "!!! This string is not utf8 !!!\"";
   return JSON_Error::utf8;
  }
 }

 return 0;
}
