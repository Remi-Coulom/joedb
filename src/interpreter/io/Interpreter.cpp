#include "Interpreter.h"
#include "Database.h"
#include "dump.h"
#include "json.h"
#include "Interpreter_Dump_Writeable.h"
#include "SQL_Dump_Writeable.h"
#include "Writeable.h"
#include "type_io.h"
#include "is_identifier.h"
#include "Journal_File.h"
#include "diagnostics.h"
#include "type_io.h"

#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

/////////////////////////////////////////////////////////////////////////////
joedb::Type joedb::Readonly_Interpreter::parse_type
/////////////////////////////////////////////////////////////////////////////
(
 std::istream &in,
 std::ostream &out
)
{
 std::string type_name;
 in >> type_name;

 if (type_name == "references")
 {
  std::string table_name;
  in >> table_name;
  Table_Id table_id = db.find_table(table_name);
  if (table_id)
   return Type::reference(table_id);
 }

 #define TYPE_MACRO(type, return_type, type_id, read, write)\
 if (type_name == #type_id)\
  return Type::type_id();
 #define TYPE_MACRO_NO_REFERENCE
 #include "TYPE_MACRO.h"
 #undef TYPE_MACRO_NO_REFERENCE
 #undef TYPE_MACRO

 out << "Error: unknown type\n";
 return Type();
}

/////////////////////////////////////////////////////////////////////////////
Table_Id joedb::Readonly_Interpreter::parse_table
/////////////////////////////////////////////////////////////////////////////
(
 std::istream &in,
 std::ostream &out
)
{
 std::string table_name;
 in >> table_name;
 Table_Id table_id = db.find_table(table_name);
 if (!table_id)
  out << "Error: no such table: " << table_name << '\n';
 return table_id;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Interpreter::update_value
/////////////////////////////////////////////////////////////////////////////
(
 std::istream &in,
 Table_Id table_id,
 Record_Id record_id,
 Field_Id field_id
)
{
 switch(db.get_field_type(table_id, field_id).get_type_id())
 {
  case Type::Type_Id::null:
   throw Exception("bad field");

  #define TYPE_MACRO(type, return_type, type_id, read_method, write_method)\
  case Type::Type_Id::type_id:\
  {\
   type value = joedb::read_##type_id(in);\
   db.update_##type_id(table_id, record_id, field_id, value);\
  }\
  break;
  #include "TYPE_MACRO.h"
  #undef TYPE_MACRO
 }
}

#define ERROR_CHECK(x) do\
{\
 try {x; out << "OK: " << line << '\n';}\
 catch(const Exception &e)\
 {out << "Error: " << e.what() << " (" << line << ')' << '\n';}\
}\
while(false)

/////////////////////////////////////////////////////////////////////////////
bool joedb::Readonly_Interpreter::process_command
/////////////////////////////////////////////////////////////////////////////
(
 const std::string &line,
 std::istream &iss,
 const std::string &command,
 std::ostream &out
)
{
 if (command.size() == 0 || command[0] == '#') //////////////////////////////
  return true;
 else if (command == "table") ///////////////////////////////////////////////
 {
  const size_t max_column_width = 25;

  const Table_Id table_id = parse_table(iss, out);
  if (table_id)
  {
   const auto &fields = db.get_fields(table_id);
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
   const Record_Id last_record_id = db.get_last_record_id(table_id);
   for (Record_Id record_id = 1; record_id <= last_record_id; record_id++)
    if (db.is_used(table_id, record_id))
    {
     rows++;
     id_column.push_back(record_id);
     for (auto field: fields)
     {
      std::stringstream ss;

      switch (db.get_field_type(table_id, field.first).get_type_id())
      {
       case Type::Type_Id::null:
       break;
       #define TYPE_MACRO(type, return_type, type_id, R, W)\
       case Type::Type_Id::type_id:\
        write_##type_id(ss, db.get_##type_id(table_id, record_id, field.first));\
       break;
       #include "TYPE_MACRO.h"
       #undef TYPE_MACRO
      }

      ss.flush();
      const std::string &s = ss.str();
      columns[field.first].push_back(s);
      const size_t width = utf8_display_size(s);
      if (column_width[field.first] < width)
       column_width[field.first] = width;
     }
    }

   //
   // Determine table width
   //
   size_t id_width = 0;
   {
    std::stringstream ss;
    ss << last_record_id;
    ss.flush();
    id_width = ss.str().size();
   }
   size_t table_width = id_width;
   for (auto field: fields)
   {
    if (column_width[field.first] > max_column_width)
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
    const auto type = db.get_field_type(table_id, field.first).get_type_id();
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
     const auto type = db.get_field_type(table_id, field.first).get_type_id();
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
 else if (command == "schema") //////////////////////////////////////////////
 {
  Interpreter_Dump_Writeable dump_writeable(out);
  dump(db, dump_writeable, true);
 }
 else if (command == "dump") ////////////////////////////////////////////////
 {
  Interpreter_Dump_Writeable dump_writeable(out);
  dump(db, dump_writeable);
 }
 else if (command == "sql") /////////////////////////////////////////////////
 {
  SQL_Dump_Writeable dump_writeable(out);
  dump(db, dump_writeable);
 }
 else if (command == "json") ////////////////////////////////////////////////
 {
  bool use_base64 = false;
  iss >> use_base64;
  write_json(out, db, use_base64);
 }
 else if (command == "help") ////////////////////////////////////////////////
 {
  out << '\n';
  out << "Commands unrelated to the database\n";
  out << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
  out << " about\n";
  out << " help\n";
  out << " quit\n";
  out << '\n';
  out << "Displaying data\n";
  out << "~~~~~~~~~~~~~~~\n";
  out << " table <table_name>\n";
  out << " schema\n";
  out << " dump\n";
  out << " sql\n";
  out << " json [<base64>]\n";
  out << '\n';
 }
 else if (command == "about") ///////////////////////////////////////////////
 {
  about_joedb(out);
 }
 else if (command == "quit") ////////////////////////////////////////////////
  return false;
 else
  out << "Error: unknown command: " << command << ". For a list of available commands, try \"help\".\n";

 return true;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Interpreter::process_command
/////////////////////////////////////////////////////////////////////////////
(
 const std::string &line,
 std::istream &iss,
 const std::string &command,
 std::ostream &out
)
{
 if (command == "help") ////////////////////////////////////////////////
 {
  Readonly_Interpreter::process_command(line, iss, command, out);
  out << "Logging\n";
  out << "~~~~~~~\n";
  out << " timestamp [<stamp>] (if no value is given, use current time)\n";
  out << " comment \"<comment_string>\"\n";
  out << " valid_data\n";
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
 else if (command == "create_table") ////////////////////////////////////////
 {
  std::string table_name;
  iss >> table_name;
  ERROR_CHECK(db.create_table(table_name));
 }
 else if (command == "drop_table") //////////////////////////////////////////
 {
  const Table_Id table_id = parse_table(iss, out);
  ERROR_CHECK(db.drop_table(table_id));
 }
 else if (command == "rename_table") ////////////////////////////////////////
 {
  const Table_Id table_id = parse_table(iss, out);
  std::string new_name;
  iss >> new_name;
  ERROR_CHECK(db.rename_table(table_id, new_name));
 }
 else if (command == "add_field") ///////////////////////////////////////////
 {
  const Table_Id table_id = parse_table(iss, out);
  std::string field_name;
  iss >> field_name;
  Type type = parse_type(iss, out);
  if (type.get_type_id() != Type::Type_Id::null)
   ERROR_CHECK(db.add_field(table_id, field_name, type));
 }
 else if (command == "drop_field") /////////////////////////////////////////
 {
  const Table_Id table_id = parse_table(iss, out);
  std::string field_name;
  iss >> field_name;
  Field_Id field_id = db.find_field(table_id, field_name);
  ERROR_CHECK(db.drop_field(table_id, field_id));
 }
 else if (command == "rename_field") ///////////////////////////////////////
 {
  const Table_Id table_id = parse_table(iss, out);
  std::string field_name;
  iss >> field_name;
  Field_Id field_id = db.find_field(table_id, field_name);
  std::string new_field_name;
  iss >> new_field_name;
  ERROR_CHECK(db.rename_field(table_id, field_id, new_field_name));
 }
 else if (command == "custom") /////////////////////////////////////////////
 {
  std::string name;
  iss >> name;
  ERROR_CHECK(db.custom(name));
 }
 else if (command == "comment") ////////////////////////////////////////////
 {
  const std::string comment = joedb::read_string(iss);
  ERROR_CHECK(db.comment(comment));
 }
 else if (command == "timestamp") //////////////////////////////////////////
 {
  int64_t timestamp = 0;
  iss >> timestamp;
  if (iss.fail())
   timestamp = std::time(0);
  ERROR_CHECK(db.timestamp(timestamp));
 }
 else if (command == "valid_data") /////////////////////////////////////////
 {
  ERROR_CHECK(db.valid_data());
 }
 else if (command == "insert_into") ////////////////////////////////////////
 {
  const Table_Id table_id = parse_table(iss, out);
  Record_Id record_id = 0;
  iss >> record_id;
  ERROR_CHECK(
   db.insert_into(table_id, record_id);
   if (iss.good())
    for (const auto &field: db.get_fields(table_id))
    {
     update_value(iss, table_id, record_id, field.first);
     if (iss.fail())
     {
      out << "Error: failed parsing value\n";
      break;
     }
    }
  );
 }
 else if (command == "insert_vector") //////////////////////////////////////
 {
  const Table_Id table_id = parse_table(iss, out);
  Record_Id record_id = 0;
  Record_Id size = 0;
  iss >> record_id >> size;
  ERROR_CHECK(db.insert_vector(table_id, record_id, size));
 }
 else if (command == "update") /////////////////////////////////////////////
 {
  const Table_Id table_id = parse_table(iss, out);
  Record_Id record_id = 0;
  iss >> record_id;
  std::string field_name;
  iss >> field_name;
  Field_Id field_id = db.find_field(table_id, field_name);
  ERROR_CHECK(update_value(iss, table_id, record_id, field_id));
 }
 else if (command == "update_vector") //////////////////////////////////////
 {
  const Table_Id table_id = parse_table(iss, out);
  Record_Id record_id = 0;
  iss >> record_id;
  std::string field_name;
  iss >> field_name;
  Field_Id field_id = db.find_field(table_id, field_name);
  Record_Id size = 0;
  iss >> size;

  if (max_record_id != 0 && size >= max_record_id)
   out << "Error: vector is too big\n";
  else
  {
   switch(db.get_field_type(table_id, field_id).get_type_id())
   {
    case Type::Type_Id::null:
     out << "Error: bad field\n";
    break;

    #define TYPE_MACRO(type, return_type, type_id, R, W)\
    case Type::Type_Id::type_id:\
    {\
     std::vector<type> v(size);\
     for (size_t i = 0; i < size; i++)\
      v[i] = joedb::read_##type_id(iss);\
     ERROR_CHECK(db.update_vector_##type_id(table_id, record_id, field_id, size, &v[0]));\
    }\
    break;
    #include "TYPE_MACRO.h"
    #undef TYPE_MACRO
   }
  }
 }
 else if (command == "delete_from") ////////////////////////////////////////
 {
  const Table_Id table_id = parse_table(iss, out);
  Record_Id record_id = 0;
  iss >> record_id;
  ERROR_CHECK(db.delete_from(table_id, record_id));
 }
 else
  return Readonly_Interpreter::process_command(line, iss, command, out);

 return true;
}

#undef ERROR_CHECK

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Interpreter::main_loop
/////////////////////////////////////////////////////////////////////////////
(
 std::istream &in,
 std::ostream &out
)
{
 std::string line;

 while(std::getline(in, line))
 {
  std::istringstream iss(line);
  std::string command;
  iss >> command;

  if (!process_command(line, iss, command, out))
   break;
 }
}
