#include "Interpreter.h"
#include "Database.h"
#include "dump.h"
#include "Listener.h"
#include "string_io.h"

#include <iostream>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
joedb::Type joedb::Interpreter::parse_type(std::istream &in,
                                           std::ostream &out)
{
 std::string type_name;
 in >> type_name;
 if (type_name == "string")
  return Type::string();
 if (type_name == "int32")
  return Type::int32();
 if (type_name == "int64")
  return Type::int64();
 if (type_name == "references")
 {
  std::string table_name;
  in >> table_name;
  table_id_t table_id = db.find_table(table_name);
  if (table_id)
   return Type::reference(table_id);
 }

 out << "Error: unknown type\n";
 return Type();
}

/////////////////////////////////////////////////////////////////////////////
table_id_t joedb::Interpreter::parse_table(std::istream &in,
                                           std::ostream &out)
{
 std::string table_name;
 in >> table_name;
 table_id_t table_id = db.find_table(table_name);
 if (!table_id)
  out << "Error: no such table: " << table_name << '\n';
 return table_id;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Interpreter::update_value(std::istream &in,
                                      table_id_t table_id,
                                      record_id_t record_id,
                                      field_id_t field_id)
{
 switch(db.get_field_type(table_id, field_id))
 {
  case Type::type_id_t::null:
  return false;

  case Type::type_id_t::string:
  {
   std::string value = joedb::read_string(in);
   return db.update_string(table_id, record_id, field_id, value);
  }

#define UPDATE_CASE(cpp_type, type_name)\
  case Type::type_id_t::type_name:\
  {\
   cpp_type value;\
   in >> value;\
   return db.update_##type_name(table_id, record_id, field_id, value);\
  }

  UPDATE_CASE(int32_t, int32);
  UPDATE_CASE(int64_t, int64);
  UPDATE_CASE(record_id_t, reference);
#undef UPDATE_CASE
 }

 return false;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Interpreter::main_loop(std::istream &in, std::ostream &out)
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
   dump(out, db);
  else if (command == "quit") ////////////////////////////////////////////////
   break;
  else if (command == "create_table") ////////////////////////////////////////
  {
   std::string table_name;
   iss >> table_name;
   table_id_t table_id = db.create_table(table_name);
   out << "OK: create table " << table_name <<
          "; table_id = " << table_id << '\n';
  }
  else if (command == "drop_table") //////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   if (table_id && db.drop_table(table_id))
    out << "OK: dropped table " << table_id << '\n';
   else
    out << "Error: could not drop table\n";
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
  else if (command == "insert_into") /////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   if (table_id)
   {
    record_id_t record_id = 0;
    iss >> record_id;
    if (!db.insert_into(table_id, record_id))
     out << "Error: could not insert with id: " << record_id << '\n';
    else
    {
     out << "OK: inserted record, id = " << record_id << '\n';
     if (iss.good())
      for (const auto &field:
           db.get_tables().find(table_id)->second.get_fields())
       update_value(iss, table_id, record_id, field.first);
    }
   }
  }
  else if (command == "update") //////////////////////////////////////////////
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
  else if (command == "delete_from") /////////////////////////////////////////
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
   out << "Error: unknown command: " << command << '\n';
 }
}
