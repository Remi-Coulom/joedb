#include "joedb/ui/json.h"
#include "joedb/ui/type_io.h"
#include "joedb/ui/base64.h"
#include "joedb/Readable.h"
#include "joedb/error/Exception.h"

#include <iostream>
#include <cmath>
#include <vector>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 int write_json
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
  for (const auto &[tid, tname]: db.get_tables())
  {
   const Record_Id size = db.get_size(tid);
   std::vector<int64_t> &v = reference_translation[tid];
   v.resize(size_t(size));
   int64_t position = 0;
   for (Record_Id record_id{0}; record_id < size; ++record_id)
   {
    if (!db.is_used(tid, record_id))
     v[size_t(record_id)] = -1;
    else
     v[size_t(record_id)] = position++;
   }
   table_size[tid] = position;
  }

  //
  // Write output
  //
  out << "{\n";

  bool first_table = true;
  for (const auto &[tid, tname]: db.get_tables())
  {
   const Record_Id size = db.get_size(tid);

   if (first_table)
    first_table = false;
   else
    out << ",\n";

   out << " \"" << tname << "\":\n {\n";

   out << "  \"__size\": " << table_size[tid];

   for (const auto &[fid, fname]: db.get_fields(tid))
   {
    const Type &type = db.get_field_type(tid, fid);
    out << ",\n";

    out << "  \"" << fname << "\": [";

    bool first_value = true;
    for (Record_Id record_id{0}; record_id < size; ++record_id)
    {
     if (db.is_used(tid, record_id))
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
        Record_Id reference = db.get_reference(tid, record_id, fid);
        const auto it = reference_translation.find(type.get_table_id());
        if (it != reference_translation.end())
        {
         const std::vector<int64_t> &v = it->second;
         if (to_underlying(reference) < 0 || size_t(reference) >= v.size())
          out << -1;
         else
          out << v[size_t(reference)];
        }
        else
         out << to_underlying(reference); // reference to a missing table
       }
       break;

       case Type::Type_Id::string:
       {
        const std::string &s = db.get_string(tid, record_id, fid);
        result |= write_json_string(out, s, base64);
       }
       break;

       case Type::Type_Id::blob:
       {
        const Blob blob = db.get_blob(tid, record_id, fid);
        out << '[' << blob.get_position() << ',' << blob.get_size() << ']';
       }
       break;

       #define TYPE_MACRO(type, return_type, type_id, R, W)\
       case Type::Type_Id::type_id:\
       {\
        const auto x = db.get_##type_id(tid, record_id, fid);\
        if (std::isnan(x) || std::isinf(x))\
        {\
         out << '0';\
         result |= JSON_Error::infnan;\
        }\
        else\
         write_##type_id(out, x);\
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
        const auto x = db.get_##type_id(tid, record_id, fid);\
        write_##type_id(out, x);\
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
 int write_json_string
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
    write_string(out, s, true);
   }
   catch (const Exception &)
   {
    out << "!!! This string is not utf8 !!!\"";
    return JSON_Error::utf8;
   }
  }

  return 0;
 }
}
