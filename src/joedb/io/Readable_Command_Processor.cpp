#include "joedb/io/Readable_Command_Processor.h"
#include "joedb/io/type_io.h"
#include "joedb/io/Interpreter_Dump_Writable.h"
#include "joedb/io/SQL_Dump_Writable.h"
#include "joedb/io/json.h"
#include "joedb/io/dump.h"
#include "joedb/Readable.h"
#include "joedb/journal/File.h"

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
  std::ostream &out
 ) const
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

    for (const auto &[fid, fname]: fields)
    {
     const size_t width = fname.size();
     column_width[fid] = width;
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
     ss << last_record_id;
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
       type_id == Type::Type_Id::string || type_id == Type::Type_Id::blob
      );
     }
     out << '\n';
    }
   }
  }
  else if (command == "record") /////////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, out);
   Record_Id record_id;
   if (!(parameters >> record_id))
    record_id = Record_Id{1};
   if (!readable.is_used(table_id, record_id))
    throw Exception("no such record");

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
  else if (command == "table_size") /////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, out);
   const auto &freedom = readable.get_freedom(table_id);
   out << freedom.get_used_count() << '\n';
  }
  else if (blob_reader && command == "read_blob") ///////////////////////////
  {
   const Blob blob = read_blob(parameters);

   if (!blob.is_null())
   {
    const std::string s = blob_reader->read_blob_data(blob);
    const std::string file_name = read_string(parameters);

    if (file_name.empty())
    {
     write_string(out, s);
     out << '\n';
    }
    else
    {
     File file(file_name, joedb::Open_Mode::create_new);
     file.write_data(s.data(), s.size());
     file.flush();
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
 table_size <table_name>
 schema
 dump
 sql
 json [<base64>]
)RRR";

   if (blob_reader)
    out << " read_blob <blob> [<output_file_name>]\n";

   out << '\n';

   return Status::ok;
  }
  else
   return Status::not_found;

  return Status::done;
 }
}
