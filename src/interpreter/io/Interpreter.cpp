#include "Interpreter.h"
#include "Database.h"
#include "dump.h"
#include "Listener.h"

#include <iostream>
#include <sstream>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
Type Interpreter::parse_type(std::istream &in, std::ostream &out)
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
table_id_t Interpreter::parse_table(std::istream &in, std::ostream &out)
{
 std::string table_name;
 in >> table_name;
 table_id_t table_id = db.find_table(table_name);
 if (!table_id)
  out << "Error: no such table: " << table_name << '\n';
 return table_id;
}

/////////////////////////////////////////////////////////////////////////////
Value Interpreter::parse_value(Type::type_id_t type_id, std::istream &in)
{
 Value value;

 switch(type_id)
 {
  case Type::type_id_t::null:
  break;

  case Type::type_id_t::string:
  {
   std::string s;
   in >> s;
   value = Value(s);
  }
  break;

  case Type::type_id_t::reference:
  {
   record_id_t record_id = 0;
   in >> record_id;
   value = Value(record_id);
  }
  break;

  case Type::type_id_t::int32:
  {
   int32_t v;
   in >> v;
   value = Value(v);
  }
  break;

  case Type::type_id_t::int64:
  {
   int64_t v;
   in >> v;
   value = Value(v);
  }
  break;
 }

 return value;
}

/////////////////////////////////////////////////////////////////////////////
void Interpreter::main_loop(std::istream &in, std::ostream &out)
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
     for (const auto &field:
          db.get_tables().find(table_id)->second.get_fields())
     {
      Value value = parse_value(field.second.type.get_type_id(), iss);
      db.update(table_id, record_id, field.first, value);
     }
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
     Value value = parse_value(db.get_field_type(table_id, field_id),
                                      iss);
     if (db.update(table_id, record_id, field_id, value))
      out << "OK: updated\n";
     else
      out << "Error: could not find record: " << record_id << '\n';
    }
   }
  }
  else if (command == "delete") //////////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(iss, out);
   if (table_id)
   {
    record_id_t record_id = 0;
    iss >> record_id;
    if (db.delete_record(table_id, record_id))
     out << "OK: record number " << record_id << " deleted\n";
    else
     out << "Error: could not delete record " << record_id << '\n';
   }
  }
  else
   out << "Error: unknown command: " << command << '\n';
 }
}
