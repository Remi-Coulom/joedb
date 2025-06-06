#include "joedb/ui/Readable_Command_Processor.h"
#include "joedb/ui/type_io.h"
#include "joedb/ui/Interpreter_Dump_Writable.h"
#include "joedb/ui/SQL_Dump_Writable.h"
#include "joedb/ui/json.h"
#include "joedb/ui/dump.h"
#include "joedb/Readable.h"

#include <vector>
#include <sstream>
#include <iomanip>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Table_Id Readable_Command_Processor::parse_table
 ////////////////////////////////////////////////////////////////////////////
 (
  std::istream &in,
  const Readable &readable
 )
 {
  std::string table_name;
  in >> table_name;
  const Table_Id table_id = readable.find_table(table_name);
  if (table_id == Table_Id(0))
   throw Exception("No such table: " + table_name);
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
   const Table_Id table_id = parse_table(parameters, readable);

   size_t max_column_width = 25;
   {
    size_t w;
    if (parameters >> w)
     max_column_width = w;
   }

   Record_Id start = Record_Id{0};
   size_t length = 0;

   parameters >> start >> length;

   if (table_id != Table_Id(0))
   {
    const auto &fields = readable.get_fields(table_id);
    std::map<Field_Id, size_t> column_width;

    for (const auto &[fid, fname]: fields)
     column_width[fid] = fname.size();

    //
    // Store values in strings to determine column widths
    //
    std::map<Field_Id, std::vector<std::string>> columns;
    std::vector<Record_Id> id_column;

    size_t rows = 0;
    const Record_Id size = readable.get_size(table_id);
    for (Record_Id record_id{0}; record_id < size; ++record_id)
    {
     if
     (
      readable.is_used(table_id, record_id) &&
      (length == 0 || (record_id >= start && record_id < start + length))
     )
     {
      rows++;
      id_column.emplace_back(record_id);

      for (const auto &[fid, fname]: fields)
      {
       std::ostringstream ss;
       write_value(ss, table_id, record_id, fid);
       ss.flush();
       const std::string &s = ss.str();
       const size_t width = utf8_display_size(s);
       if (column_width[fid] < width)
        column_width[fid] = width;
       columns[fid].emplace_back(s);
      }
     }
    }

    //
    // Determine table width
    //
    size_t id_width = 0;
    {
     std::ostringstream ss;
     ss << size - 1;
     ss.flush();
     id_width = ss.str().size();
    }
    size_t table_width = id_width;
    for (const auto &[fid, fname]: fields)
    {
     if (max_column_width && column_width[fid] > max_column_width)
      column_width[fid] = max_column_width;
     table_width += column_width[fid] + 1;
    }

    //
    // Table header
    //
    out << std::string(table_width, '-') << '\n';
    out << std::string(id_width, ' ');
    for (const auto &[fid, fname]: fields)
    {
     const Type::Type_Id type_id = readable.get_field_type
     (
      table_id,
      fid
     ).get_type_id();
     out << ' ';
     write_justified
     (
      out,
      fname,
      column_width[fid],
      type_id == Type::Type_Id::string
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

     for (const auto &[fid, fname]: fields)
     {
      const Type::Type_Id type_id = readable.get_field_type
      (
       table_id,
       fid
      ).get_type_id();

      out << ' ';
      write_justified
      (
       out,
       columns[fid][i],
       column_width[fid],
       type_id == Type::Type_Id::string
      );
     }
     out << '\n';
    }
   }
  }
  else if (command == "record") /////////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, readable);
   Record_Id record_id;
   if (!(parameters >> record_id))
    record_id = Record_Id::null;

   const auto &freedom = readable.get_freedom(table_id);

   if (!readable.is_used(table_id, record_id))
   {
    if (record_id.is_not_null())
     out << record_id << " is not used.\n";
    out << "first_used: " << freedom.get_first_used() << '\n';
    out << "last_used: " << freedom.get_last_used() << '\n';
    out << "used_count: " << freedom.get_used_count() << '\n';
    out << "size: " << freedom.get_size() << '\n';
    out << "dense: " << freedom.is_dense() << '\n';
   }
   else
   {
    out << "id = " << record_id;
    out << "; next = " << freedom.get_next(record_id);
    out << "; previous = " << freedom.get_previous(record_id) << '\n';

    const auto &fields = readable.get_fields(table_id);
    size_t max_field_size = 0;
    for (const auto &[fid, fname]: fields)
     if (fname.size() > max_field_size)
      max_field_size = fname.size();

    for (const auto &[fid, fname]: fields)
    {
     out << std::setw(int(max_field_size)) << fname << ": ";
     write_value(out, table_id, record_id, fid);
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
 record <table_name> [<record_id>]
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
