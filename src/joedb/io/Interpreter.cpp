#include "joedb/Writable.h"
#include "joedb/is_identifier.h"
#include "joedb/io/Interpreter.h"
#include "joedb/interpreter/Database.h"
#include "joedb/io/dump.h"
#include "joedb/io/json.h"
#include "joedb/io/Interpreter_Dump_Writable.h"
#include "joedb/io/SQL_Dump_Writable.h"
#include "joedb/io/type_io.h"
#include "joedb/journal/diagnostics.h"
#include "type_io.h"

#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Type Readonly_Interpreter::parse_type
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
   if (table_id)
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
 Table_Id Readonly_Interpreter::parse_table
 ////////////////////////////////////////////////////////////////////////////
 (
  std::istream &in,
  std::ostream &out
 ) const
 {
  std::string table_name;
  in >> table_name;
  const Table_Id table_id = readable.find_table(table_name);
  if (!table_id)
  {
   std::ostringstream error;
   error << "No such table: " << table_name;
   throw Exception(error.str());
  }
  return table_id;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Readonly_Interpreter::after_command
 ////////////////////////////////////////////////////////////////////////////
 (
  std::ostream &out,
  int64_t line_number,
  const std::string &line,
  const Exception *exception
 ) const
 {
  if (exception)
  {
   std::ostringstream error;
   error << exception->what();
   error << "\nLine " << line_number << ": " << line << '\n';

   if (rethrow)
    throw Exception(error.str());
   else
    out << "Error: " << error.str();
  }
  else if (echo)
   out << "OK: " << line << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter::update_value
 ////////////////////////////////////////////////////////////////////////////
 (
  std::istream &in,
  Table_Id table_id,
  Record_Id record_id,
  Field_Id field_id
 )
 {
  switch(readable.get_field_type(table_id, field_id).get_type_id())
  {
   case Type::Type_Id::null:
    throw Exception("bad field");

   #define TYPE_MACRO(type, return_type, type_id, read_method, write_method)\
   case Type::Type_Id::type_id:\
   {\
    const type value = joedb::read_##type_id(in);\
    writable.update_##type_id(table_id, record_id, field_id, value);\
   }\
   break;
   #include "joedb/TYPE_MACRO.h"
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 bool Readonly_Interpreter::process_command
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::string &command,
  std::istream &iss,
  std::ostream &out
 )
 {
  if (command.empty() || command[0] == '#') /////////////////////////////////
   return true;
  else if (command == "table") //////////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);

   size_t max_column_width = 25;
   {
    size_t w;
    if (iss >> w)
     max_column_width = w;
   }

   size_t start = 0;
   size_t length = 0;

   iss >> start >> length;

   if (table_id)
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
    for (Record_Id record_id = 1; record_id <= last_record_id; record_id++)
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
        #define TYPE_MACRO(type, return_type, type_id, R, W)\
        case Type::Type_Id::type_id:\
         write_##type_id(ss, readable.get_##type_id(table_id, record_id, field.first));\
        break;
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
     const auto type = readable.get_field_type(table_id, field.first).get_type_id();
     out << ' ';
     write_justified
     (
      out,
      field.second,
      column_width[field.first],
      type == Type::Type_Id::string
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
      const auto type = readable.get_field_type(table_id, field.first).get_type_id();
      out << ' ';
      write_justified
      (
       out,
       columns[field.first][i],
       column_width[field.first],
       type == Type::Type_Id::string
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
   iss >> use_base64;
   write_json(out, readable, use_base64);
  }
  else if (command == "help") ///////////////////////////////////////////////
  {
   out << '\n';
   out << "Commands unrelated to the database\n";
   out << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
   out << " about\n";
   out << " help\n";
   out << " quit\n";
   out << " echo on|off\n";
   out << '\n';
   out << "Displaying data\n";
   out << "~~~~~~~~~~~~~~~\n";
   out << " table <table_name> [<max_column_width>] [start] [length]\n";
   out << " schema\n";
   out << " dump\n";
   out << " sql\n";
   out << " json [<base64>]\n";
   out << '\n';
  }
  else if (command == "about") //////////////////////////////////////////////
  {
   about_joedb(out);
  }
  else if (command == "echo") ///////////////////////////////////////////////
  {
   std::string parameter;
   iss >> parameter;

   if (parameter == "on")
    set_echo(true);
   else if (parameter == "off")
    set_echo(false);
  }
  else if (command == "quit") ///////////////////////////////////////////////
   return false;
  else
   throw Exception("Unknown command. For a list of available commands, try \"help\".");

  return true;
 }

 ////////////////////////////////////////////////////////////////////////////
 bool Interpreter::process_command
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::string &command,
  std::istream &iss,
  std::ostream &out
 )
 {
  if (command == "help") ////////////////////////////////////////////////////
  {
   Readonly_Interpreter::process_command(command, iss, out);
   out << "Logging\n";
   out << "~~~~~~~\n";
   out << " timestamp [<stamp>] (if no value is given, use current time)\n";
   out << " comment \"<comment_string>\"\n";
   out << " valid_data\n";
   out << " checkpoint\n";
   out << '\n';
   out << "Data definition\n";
   out << "~~~~~~~~~~~~~~~\n";
   out << " create_table <table_name>\n";
   out << " drop_table <table_name>\n";
   out << " rename_table <old_table_name> <new_table_name>\n";
   out << " add_field <table_name> <field_name> <type>\n";
   out << " drop_field <table_name> <field_name>\n";
   out << " rename_field <table_name> <old_field_name> <new_field_name>\n";
   out << " custom <custom_name>\n";
   out << '\n';
   out << " <type> may be:\n";
   out << "  string,\n";
   out << "  blob,\n";
   out << "  int8, int16, int32, int64,\n";
   out << "  float32, float64,\n";
   out << "  boolean,\n";
   out << "  references <table_name>\n";
   out << '\n';
   out << "Data manipulation\n";
   out << "~~~~~~~~~~~~~~~~~\n";
   out << " insert_into <table_name> <record_id>\n";
   out << " insert_vector <table_name> <record_id> <size>\n";
   out << " update <table_name> <record_id> <field_name> <value>\n";
   out << " update_vector <table_name> <record_id> <field_name> <N> <v_1> ... <v_N>\n";
   out << " delete_from <table_name> <record_id>\n";
   out << '\n';
  }
  else if (command == "create_table") ///////////////////////////////////////
  {
   std::string table_name;
   iss >> table_name;
   writable.create_table(table_name);
  }
  else if (command == "drop_table") /////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   writable.drop_table(table_id);
  }
  else if (command == "rename_table") ///////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   std::string new_name;
   iss >> new_name;
   writable.rename_table(table_id, new_name);
  }
  else if (command == "add_field") //////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   std::string field_name;
   iss >> field_name;
   const Type type = parse_type(iss, out);
   if (type.get_type_id() != Type::Type_Id::null)
    writable.add_field(table_id, field_name, type);
  }
  else if (command == "drop_field") ////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   std::string field_name;
   iss >> field_name;
   const Field_Id field_id = readable.find_field(table_id, field_name);
   writable.drop_field(table_id, field_id);
  }
  else if (command == "rename_field") //////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   std::string field_name;
   iss >> field_name;
   const Field_Id field_id = readable.find_field(table_id, field_name);
   std::string new_field_name;
   iss >> new_field_name;
   writable.rename_field(table_id, field_id, new_field_name);
  }
  else if (command == "custom") ////////////////////////////////////////////
  {
   std::string name;
   iss >> name;
   writable.custom(name);
  }
  else if (command == "comment") ///////////////////////////////////////////
  {
   const std::string comment = joedb::read_string(iss);
   writable.comment(comment);
  }
  else if (command == "timestamp") /////////////////////////////////////////
  {
   int64_t timestamp = 0;
   iss >> timestamp;
   if (iss.fail())
    timestamp = std::time(nullptr);
   writable.timestamp(timestamp);
  }
  else if (command == "valid_data") ////////////////////////////////////////
  {
   writable.valid_data();
  }
  else if (command == "checkpoint") ////////////////////////////////////////
  {
   writable.checkpoint(Commit_Level::no_commit);
  }
  else if (command == "insert_into") ///////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);

   Record_Id record_id = 0;
   iss >> record_id;
   if (record_id == 0)
    record_id = readable.get_last_record_id(table_id) + 1;

   writable.insert_into(table_id, record_id);
   if (iss.good())
    for (const auto &field: readable.get_fields(table_id))
    {
     update_value(iss, table_id, record_id, field.first);
     if (iss.fail())
      throw Exception("failed parsing value");
    }
  }
  else if (command == "insert_vector") /////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   Record_Id record_id = 0;
   Record_Id size = 0;
   iss >> record_id >> size;
   writable.insert_vector(table_id, record_id, size);
  }
  else if (command == "update") ////////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   Record_Id record_id = 0;
   iss >> record_id;
   std::string field_name;
   iss >> field_name;
   const Field_Id field_id = readable.find_field(table_id, field_name);
   update_value(iss, table_id, record_id, field_id);
  }
  else if (command == "update_vector") /////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   Record_Id record_id = 0;
   iss >> record_id;
   std::string field_name;
   iss >> field_name;
   const Field_Id field_id = readable.find_field(table_id, field_name);
   Record_Id size = 0;
   iss >> size;

   if (max_record_id != 0 && size >= max_record_id)
    throw Exception("vector is too big");
   else
   {
    switch(readable.get_field_type(table_id, field_id).get_type_id())
    {
     case Type::Type_Id::null:
      throw Exception("bad field");
     break;

     #define TYPE_MACRO(type, return_type, type_id, R, W)\
     case Type::Type_Id::type_id:\
     {\
      std::vector<type> v(size);\
      for (size_t i = 0; i < size; i++)\
       v[i] = joedb::read_##type_id(iss);\
      writable.update_vector_##type_id(table_id, record_id, field_id, size, &v[0]);\
     }\
     break;
     #include "joedb/TYPE_MACRO.h"
    }
   }
  }
  else if (command == "delete_from") ////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   Record_Id record_id = 0;
   iss >> record_id;
   writable.delete_from(table_id, record_id);
  }
  else
   return Readonly_Interpreter::process_command(command, iss, out);

  return true;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Readonly_Interpreter::main_loop
 ////////////////////////////////////////////////////////////////////////////
 (
  std::istream &in,
  std::ostream &out
 )
 {
  int64_t line_number = 0;

  std::string line;

  while(std::getline(in, line))
  {
   line_number++;
   std::istringstream iss(line);
   std::string command;
   iss >> command;

   try
   {
    const bool again = process_command(command, iss, out);
    after_command(out, line_number, line, nullptr);
    if (!again)
     break;
   }
   catch (const Exception &e)
   {
    after_command(out, line_number, line, &e);
   }
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter::main_loop
 ////////////////////////////////////////////////////////////////////////////
 (
  std::istream &in,
  std::ostream &out
 )
 {
  Readonly_Interpreter::main_loop(in, out);
  writable.checkpoint(Commit_Level::no_commit);
 }
}
