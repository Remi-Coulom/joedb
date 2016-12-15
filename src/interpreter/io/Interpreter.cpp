#include "Interpreter.h"
#include "joedb/Database.h"
#include "dump.h"
#include "Interpreter_Dump_Writeable.h"
#include "joedb/Writeable.h"
#include "type_io.h"
#include "is_identifier.h"
#include "joedb/Journal_File.h"
#include "diagnostics.h"

#include <iostream>
#include <sstream>
#include <ctime>

/////////////////////////////////////////////////////////////////////////////
joedb::Type joedb::Interpreter::parse_type
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
  table_id_t table_id = db.find_table(table_name);
  if (table_id)
   return Type::reference(table_id);
 }

 #define TYPE_MACRO(type, return_type, type_id, read, write)\
 if (type_name == #type_id)\
  return Type::type_id();
 #define TYPE_MACRO_NO_REFERENCE
 #include "joedb/TYPE_MACRO.h"
 #undef TYPE_MACRO_NO_REFERENCE
 #undef TYPE_MACRO

 out << "Error: unknown type\n";
 return Type();
}

/////////////////////////////////////////////////////////////////////////////
table_id_t joedb::Interpreter::parse_table
/////////////////////////////////////////////////////////////////////////////
(
 std::istream &in,
 std::ostream &out
)
{
 std::string table_name;
 in >> table_name;
 table_id_t table_id = db.find_table(table_name);
 if (!table_id)
  out << "Error: no such table: " << table_name << '\n';
 return table_id;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Interpreter::update_value
/////////////////////////////////////////////////////////////////////////////
(
 std::istream &in,
 table_id_t table_id,
 record_id_t record_id,
 field_id_t field_id
)
{
 switch(db.get_field_type(table_id, field_id))
 {
  case Type::type_id_t::null:
   throw std::runtime_error("bad field");

  #define TYPE_MACRO(type, return_type, type_id, read_method, write_method)\
  case Type::type_id_t::type_id:\
  {\
   type value = joedb::read_##type_id(in);\
   db.update_##type_id(table_id, record_id, field_id, value);\
  }\
  break;
  #include "joedb/TYPE_MACRO.h"
  #undef TYPE_MACRO
 }
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Interpreter::main_loop(std::istream &in, std::ostream &out)
/////////////////////////////////////////////////////////////////////////////
{
 std::string line;

 while(std::getline(in, line))
 {
  std::istringstream iss(line);
  std::string command;
  iss >> command;

  #define ERROR_CHECK(x) do\
  {\
   try {x; out << "OK: " << line << '\n';}\
   catch(std::runtime_error &e) {out << "Error: " << e.what() << " (" << line << ')' << '\n';}\
  }\
  while(false)

#if 0
  out << "running: " << line << '\n';
#endif

  if (command.size() == 0 || command[0] == '#') //////////////////////////////
   continue;
  else if (command == "dump") ////////////////////////////////////////////////
  {
   Interpreter_Dump_Writeable dump_writeable(out);
   dump(db, dump_writeable);
  }
  else if (command == "quit") ////////////////////////////////////////////////
   break;
  else if (command == "help") ////////////////////////////////////////////////
  {
   out << '\n';
   out << "Commands that don't modify the database\n";
   out << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
   out << " about\n";
   out << " help\n";
   out << " dump\n";
   out << " quit\n";
   out << '\n';
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
  else if (command == "about") ///////////////////////////////////////////////
  {
   about_joedb(out);
  }
  else if (command == "create_table") ////////////////////////////////////////
  {
   std::string table_name;
   iss >> table_name;
   ERROR_CHECK(db.create_table(table_name));
  }
  else if (command == "drop_table") //////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   ERROR_CHECK(db.drop_table(table_id));
  }
  else if (command == "rename_table") ////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   std::string new_name;
   iss >> new_name;
   ERROR_CHECK(db.rename_table(table_id, new_name));
  }
  else if (command == "add_field") ///////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   std::string field_name;
   iss >> field_name;
   Type type = parse_type(iss, out);
   if (type.get_type_id() != Type::type_id_t::null)
    ERROR_CHECK(db.add_field(table_id, field_name, type));
  }
  else if (command == "drop_field") /////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   std::string field_name;
   iss >> field_name;
   field_id_t field_id = db.find_field(table_id, field_name);
   ERROR_CHECK(db.drop_field(table_id, field_id));
  }
  else if (command == "rename_field") ///////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   std::string field_name;
   iss >> field_name;
   field_id_t field_id = db.find_field(table_id, field_name);
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
   const table_id_t table_id = parse_table(iss, out);
   record_id_t record_id = 0;
   iss >> record_id;
   ERROR_CHECK(
    db.insert(table_id, record_id);
    if (iss.good())
     for (const auto &field:
          db.get_tables().find(table_id)->second.get_fields())
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
   const table_id_t table_id = parse_table(iss, out);
   record_id_t record_id = 0;
   record_id_t size = 0;
   iss >> record_id >> size;
   ERROR_CHECK(db.insert_vector(table_id, record_id, size));
  }
  else if (command == "update") /////////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   record_id_t record_id = 0;
   iss >> record_id;
   std::string field_name;
   iss >> field_name;
   field_id_t field_id = db.find_field(table_id, field_name);
   ERROR_CHECK(update_value(iss, table_id, record_id, field_id));
  }
  else if (command == "update_vector") //////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   record_id_t record_id = 0;
   iss >> record_id;
   std::string field_name;
   iss >> field_name;
   field_id_t field_id = db.find_field(table_id, field_name);
   record_id_t size = 0;
   iss >> size;
   const Type::type_id_t field_type = db.get_field_type(table_id, field_id);

   switch(field_type)
   {
    case Type::type_id_t::null:
     out << "Error: bad field\n";
    break;

    #define TYPE_MACRO(type, return_type, type_id, R, W)\
    case Type::type_id_t::type_id:\
    {\
     std::vector<type> v(size);\
     for (size_t i = 0; i < size; i++)\
      v[i] = joedb::read_##type_id(iss);\
     ERROR_CHECK(db.update_vector_##type_id(table_id, record_id, field_id, size, &v[0]));\
    }\
    break;
    #include "joedb/TYPE_MACRO.h"
    #undef TYPE_MACRO
   }
  }
  else if (command == "delete_from") ////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   record_id_t record_id = 0;
   iss >> record_id;
   ERROR_CHECK(db.delete_record(table_id, record_id));
  }
  else
   out << "Error: unknown command: " << command << ". For a list of available commands, try \"help\".\n";
 }

 #undef ERROR_CHECK
}
