#include "joedb/io/Readable_Command_Processor.h"
#include "joedb/io/type_io.h"
#include "joedb/io/Interpreter_Dump_Writable.h"
#include "joedb/io/SQL_Dump_Writable.h"
#include "joedb/io/json.h"
#include "joedb/io/dump.h"
#include "joedb/Readable.h"

#include <vector>
#include <sstream>
#include <iomanip>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Type Readable_Command_Processor::parse_type
 ////////////////////////////////////////////////////////////////////////////
 (
  std::istream &in,
  std::ostream &out
 ) const
 {
  std::string type_name;
  in >> type_name;

  if (type_name == "references")
  {
   std::string table_name;
   in >> table_name;
   const Table_Id table_id = readable.find_table(table_name);
   if (table_id != Table_Id(0))
    return Type::reference(table_id);
  }

  #define TYPE_MACRO(type, return_type, type_id, read, write)\
  if (type_name == #type_id)\
   return Type::type_id();
  #define TYPE_MACRO_NO_REFERENCE
  #include "joedb/TYPE_MACRO.h"

  throw Exception("unknown type");
 }

 ////////////////////////////////////////////////////////////////////////////
 Table_Id Readable_Command_Processor::parse_table
 ////////////////////////////////////////////////////////////////////////////
 (
  std::istream &in,
  std::ostream &out
 ) const
 {
  std::string table_name;
  in >> table_name;
  const Table_Id table_id = readable.find_table(table_name);
  if (table_id == Table_Id(0))
  {
   std::ostringstream error;
   error << "No such table: " << table_name;
   throw Exception(error.str());
  }
  return table_id;
 }

 ////////////////////////////////////////////////////////////////////////////
 Command_Processor::Status Readable_Command_Processor::process_command
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::string &command,
  std::istream &parameters,
  std::istream &in,
  std::ostream &out
 )
 {
  if (command == "table") ///////////////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, out);

   size_t max_column_width = 25;
   {
    size_t w;
    if (parameters >> w)
     max_column_width = w;
   }

   Record_Id start = Record_Id(0);
   size_t length = 0;

   parameters >> start >> length;

   if (table_id != Table_Id(0))
   {
    const auto &fields = readable.get_fields(table_id);
    std::map<Field_Id, size_t> column_width;

    for (auto field: fields)
    {
     size_t width = field.second.size();
     column_width[field.first] = width;
    }

    //
    // Store values in strings to determine column widths
    //
    std::map<Field_Id, std::vector<std::string>> columns;
    std::vector<Record_Id> id_column;

    size_t rows = 0;
    const Record_Id last_record_id = readable.get_last_record_id(table_id);
    for (Record_Id record_id = Record_Id(1); record_id <= last_record_id; ++record_id)
    {
     if
     (
      readable.is_used(table_id, record_id) &&
      (length == 0 || (record_id >= start && record_id < start + length))
     )
     {
      rows++;
      id_column.emplace_back(record_id);
      for (auto field: fields)
      {
       std::ostringstream ss;

       switch (readable.get_field_type(table_id, field.first).get_type_id())
       {
        case Type::Type_Id::null:
        break;

        case Type::Type_Id::blob:
        {
         const Blob blob = readable.get_blob
         (
          table_id,
          record_id,
          field.first
         );

         if (blob.get_position() && blob_reader)
         {
          write_string
          (
           ss,
           blob_reader->read_blob_data(blob)
          );
         }
         else
          write_blob(ss, blob);
        }
        break;

        #define TYPE_MACRO(type, return_type, type_id, R, W)\
        case Type::Type_Id::type_id:\
         write_##type_id(ss, readable.get_##type_id(table_id, record_id, field.first));\
        break;
        #define TYPE_MACRO_NO_BLOB
        #include "joedb/TYPE_MACRO.h"
       }

       ss.flush();
       {
        std::string s = ss.str();
        const size_t width = utf8_display_size(s);
        if (column_width[field.first] < width)
         column_width[field.first] = width;
        columns[field.first].emplace_back(std::move(s));
       }
      }
     }
    }

    //
    // Determine table width
    //
    size_t id_width = 0;
    {
     std::ostringstream ss;
     ss << last_record_id;
     ss.flush();
     id_width = ss.str().size();
    }
    size_t table_width = id_width;
    for (auto field: fields)
    {
     if (max_column_width && column_width[field.first] > max_column_width)
      column_width[field.first] = max_column_width;
     table_width += column_width[field.first] + 1;
    }

    //
    // Table header
    //
    out << std::string(table_width, '-') << '\n';
    out << std::string(id_width, ' ');
    for (auto field: fields)
    {
     const Type::Type_Id type_id = readable.get_field_type
     (
      table_id,
      field.first
     ).get_type_id();
     out << ' ';
     write_justified
     (
      out,
      field.second,
      column_width[field.first],
      type_id == Type::Type_Id::string || type_id == Type::Type_Id::blob
     );
    }
    out << '\n';
    out << std::string(table_width, '-') << '\n';

    //
    // Table data
    //
    for (size_t i = 0; i < rows; i++)
    {
     out << std::setw(int(id_width)) << id_column[i];

     for (auto field: fields)
     {
      const Type::Type_Id type_id = readable.get_field_type
      (
       table_id,
       field.first
      ).get_type_id();

      out << ' ';
      write_justified
      (
       out,
       columns[field.first][i],
       column_width[field.first],
       type_id == Type::Type_Id::string || type_id == Type::Type_Id::blob
      );
     }
     out << '\n';
    }
   }
  }
  else if (command == "schema") /////////////////////////////////////////////
  {
   Interpreter_Dump_Writable dump_writable(out);
   dump(readable, dump_writable, true);
  }
  else if (command == "dump") ///////////////////////////////////////////////
  {
   Interpreter_Dump_Writable dump_writable(out);
   dump(readable, dump_writable);
  }
  else if (command == "sql") ////////////////////////////////////////////////
  {
   SQL_Dump_Writable dump_writable(out);
   dump(readable, dump_writable);
  }
  else if (command == "json") ///////////////////////////////////////////////
  {
   bool use_base64 = false;
   parameters >> use_base64;
   write_json(out, readable, use_base64);
  }
  else if (command == "help") ///////////////////////////////////////////////
  {
   out << R"RRR(Displaying data
~~~~~~~~~~~~~~~
 table <table_name> [<max_column_width>] [start] [length]
 schema
 dump
 sql
 json [<base64>]

)RRR";

   return Status::ok;
  }
  else
   return Status::not_found;

  return Status::done;
 }
}
