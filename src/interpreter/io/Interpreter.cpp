#include "Interpreter.h"
#include "joedb/Database.h"
#include "dump.h"
#include "Interpreter_Dump_Listener.h"
#include "joedb/Listener.h"
#include "type_io.h"
#include "is_identifier.h"
#include "joedb/Journal_File.h"
#include "diagnostics.h"

#include <iostream>
#include <sstream>
#include <ctime>

/////////////////////////////////////////////////////////////////////////////
bool joedb::Interpreter::too_big(record_id_t record_id)
/////////////////////////////////////////////////////////////////////////////
{
#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
 return record_id > 1000000;
#else
 return false;
#endif
}

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
bool joedb::Interpreter::update_value
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
  return false;

  #define TYPE_MACRO(type, return_type, type_id, read_method, write_method)\
  case Type::type_id_t::type_id:\
  {\
   type value = joedb::read_##type_id(in);\
   return db.update_##type_id(table_id, record_id, field_id, value);\
  }
  #include "joedb/TYPE_MACRO.h"
  #undef TYPE_MACRO
 }

 return false;
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

  if (command.size() == 0 || command[0] == '#') //////////////////////////////
   continue;
  else if (command == "dump") ////////////////////////////////////////////////
  {
   Interpreter_Dump_Listener dump_listener(out);
   dump(db, dump_listener);
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
   if (!is_identifier(table_name))
    out << "Error: \"" << table_name << "\" is not a valid identifier\n";
   else
   {
    table_id_t table_id = db.create_table(table_name);
    if (table_id == 0)
     out << "Error: table " << table_name << " already exists\n";
    else
     out << "OK: create table " << table_name <<
            "; table_id = " << table_id << '\n';
   }
  }
  else if (command == "drop_table") //////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   if (table_id && db.drop_table(table_id))
    out << "OK: dropped table " << table_id << '\n';
   else
    out << "Error: could not drop table\n";
  }
  else if (command == "rename_table") ////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   std::string new_name;
   iss >> new_name;
   if (db.rename_table(table_id, new_name))
    out << "OK: renamed table " << table_id << " to " << new_name << '\n';
   else
    out << "Error: could not rename table\n";
  }
  else if (command == "add_field") ///////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   if (table_id)
   {
    std::string field_name;
    iss >> field_name;
    Type type = parse_type(iss, out);
    if (type.get_type_id() != Type::type_id_t::null)
    {
     field_id_t field_id = db.add_field(table_id, field_name, type);
     if (field_id)
      out << "OK: add_field " << field_name <<
                   "; field_id = " << field_id << '\n';
     else
      out << "Error: could not add field\n";
    }
   }
  }
  else if (command == "drop_field") /////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   if (table_id)
   {
    std::string field_name;
    iss >> field_name;
    field_id_t field_id = db.find_field(table_id, field_name);
    if (!field_id)
     out << "Error: no such field: " << field_name << '\n';
    else
    {
     db.drop_field(table_id, field_id);
     out << "OK: dropped field " << field_name << '\n';
    }
   }
  }
  else if (command == "rename_field") ///////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   if (table_id)
   {
    std::string field_name;
    iss >> field_name;
    field_id_t field_id = db.find_field(table_id, field_name);
    if (!field_id)
     out << "Error: no such field: " << field_name << '\n';
    else
    {
     std::string new_field_name;
     iss >> new_field_name;
     if (db.rename_field(table_id, field_id, new_field_name))
      out << "OK: renamed field " << field_name << " to " << new_field_name << '\n';
     else
      out << "Error: could not rename this field\n";
    }
   }
  }
  else if (command == "custom") /////////////////////////////////////////////
  {
   std::string name;
   iss >> name;
   db.custom(name);
   out << "OK: custom command: " << name << '\n';
  }
  else if (command == "comment") ////////////////////////////////////////////
  {
   const std::string comment = joedb::read_string(iss);
   db.comment(comment);
   out << "OK: comment: ";
   joedb::write_string(out, comment);
   out << '\n';
  }
  else if (command == "timestamp") //////////////////////////////////////////
  {
   int64_t timestamp = 0;
   iss >> timestamp;
   out << "OK: timestamp: ";
   if (iss.fail())
   {
    db.timestamp(std::time(0));
    out << "now\n";
   }
   else
   {
    db.timestamp(timestamp);
    out << timestamp << '\n';
   }
  }
  else if (command == "valid_data") /////////////////////////////////////////
  {
   db.valid_data();
   out << "OK: valid data\n";
  }
  else if (command == "insert_into") ////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   if (table_id)
   {
    record_id_t record_id = 0;
    iss >> record_id;
    if (!record_id ||
        too_big(record_id) ||
        !db.insert_into(table_id, record_id))
     out << "Error: could not insert with id: " << record_id << '\n';
    else
    {
     out << "OK: inserted record, id = " << record_id << '\n';
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
    }
   }
  }
  else if (command == "insert_vector") //////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   if (table_id)
   {
    record_id_t record_id = 0;
    record_id_t size = 0;
    iss >> record_id >> size;
    if (!record_id ||
        !size ||
        too_big(record_id) ||
        too_big(size) ||
        !db.insert_vector(table_id, record_id, size))
     out << "Error: could not insert vector\n";
    else
    {
     out << "OK: inserted vector: record_id = " << record_id;
     out << "; size = " << size << '\n';
    }
   }
  }
  else if (command == "update") /////////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   if (table_id)
   {
    record_id_t record_id = 0;
    iss >> record_id;

    std::string field_name;
    iss >> field_name;
    field_id_t field_id = db.find_field(table_id, field_name);

    if (!field_id)
     out << "Error: no such field: " << field_name << '\n';
    else
    {
     if (update_value(iss, table_id, record_id, field_id))
      out << "OK: updated\n";
     else
      out << "Error: could not find record: " << record_id << '\n';
    }
   }
  }
  else if (command == "update_vector") //////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   if (table_id)
   {
    record_id_t record_id = 0;
    iss >> record_id;

    std::string field_name;
    iss >> field_name;
    field_id_t field_id = db.find_field(table_id, field_name);

    if (!field_id)
     out << "Error: no such field: " << field_name << '\n';
    else
    {
     record_id_t size = 0;
     iss >> size;
     bool result = false;

     switch(db.get_field_type(table_id, field_id))
     {
      case Type::type_id_t::null:
      break;

      #define TYPE_MACRO(type, return_type, type_id, R, W)\
      case Type::type_id_t::type_id:\
      {\
       std::vector<type> v(size);\
       for (size_t i = 0; i < size; i++)\
        v[i] = joedb::read_##type_id(iss);\
       result = db.update_vector_##type_id(table_id, record_id, field_id, size, &v[0]);\
      }\
      break;
      #include "joedb/TYPE_MACRO.h"
      #undef TYPE_MACRO
     }

     if (result)
      out << "OK: updated " << size << " rows\n";
     else
      out << "Error: could not update vector\n";
    }
   }
  }
  else if (command == "delete_from") ////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   if (table_id)
   {
    record_id_t record_id = 0;
    iss >> record_id;
    if (db.delete_from(table_id, record_id))
     out << "OK: record number " << record_id << " deleted\n";
    else
     out << "Error: could not delete record " << record_id << '\n';
   }
  }
  else
   out << "Error: unknown command: " << command << ". For a list of available commands, try \"help\".\n";
 }
}
