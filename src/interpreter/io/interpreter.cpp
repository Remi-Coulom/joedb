#include "Database.h"
#include "dump.h"
#include "Listener.h"

#include <iostream>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
joedb::Type parse_type(const joedb::Database &db, std::istream &in)
/////////////////////////////////////////////////////////////////////////////
{
 std::string type_name;
 in >> type_name;
 if (type_name == "string")
  return joedb::Type::string();
 if (type_name == "int32")
  return joedb::Type::int32();
 if (type_name == "int64")
  return joedb::Type::int64();
 if (type_name == "references")
 {
  std::string table_name;
  in >> table_name;
  table_id_t table_id = db.find_table(table_name);
  if (table_id)
   return joedb::Type::reference(table_id);
 }

 std::cout << "Error: unknown type\n";
 return joedb::Type();
}

/////////////////////////////////////////////////////////////////////////////
table_id_t parse_table(const joedb::Database &db, std::istream &in)
/////////////////////////////////////////////////////////////////////////////
{
 std::string table_name;
 in >> table_name;
 table_id_t table_id = db.find_table(table_name);
 if (!table_id)
  std::cout << "Error: no such table: " << table_name << '\n';
 return table_id;
}

/////////////////////////////////////////////////////////////////////////////
joedb::Value parse_value(joedb::Type::type_id_t type_id, std::istream &in)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Value value;

 switch(type_id)
 {
  case joedb::Type::null_id:
  break;

  case joedb::Type::string_id:
  {
   std::string s;
   in >> s;
   value = joedb::Value(s);
  }
  break;

  case joedb::Type::reference_id:
  {
   record_id_t record_id = 0;
   in >> record_id;
   value = joedb::Value(record_id);
  }
  break;

  case joedb::Type::int32_id:
  {
   int32_t v;
   in >> v;
   value = joedb::Value(v);
  }
  break;

  case joedb::Type::int64_id:
  {
   int64_t v;
   in >> v;
   value = joedb::Value(v);
  }
  break;
 }

 return value;
}

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Listener listener;
 joedb::Database db(listener);
 std::string line;

 while(std::getline(std::cin, line))
 {
  std::istringstream iss(line);
  std::string command;
  iss >> command;

  if (command.size() == 0 || command[0] == '#') //////////////////////////////
   continue;
  else if (command == "dump") ////////////////////////////////////////////////
   joedb::dump(std::cout, db);
  else if (command == "quit") ////////////////////////////////////////////////
   break;
  else if (command == "create_table") ////////////////////////////////////////
  {
   std::string table_name;
   iss >> table_name;
   table_id_t table_id = db.create_table(table_name);
   std::cout << "OK: create table " << table_name <<
                "; table_id = " << table_id << '\n';
  }
  else if (command == "drop_table") //////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(db, iss);
   if (table_id && db.drop_table(table_id))
    std::cout << "OK: dropped table " << table_id << '\n';
   else
    std::cout << "Error: could not drop table\n";
  }
  else if (command == "add_field") ///////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(db, iss);
   if (table_id)
   {
    std::string field_name;
    iss >> field_name;
    joedb::Type type = parse_type(db, iss);
    if (type.get_type_id() != joedb::Type::null_id)
    {
     field_id_t field_id = db.add_field(table_id, field_name, type);
     if (field_id)
      std::cout << "OK: add_field " << field_name <<
                   "; field_id = " << field_id << '\n';
     else
      std::cout << "Error: could not add field\n";
    }
   }
  }
  else if (command == "drop_field") /////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(db, iss);
   if (table_id)
   { std::string field_name;
    iss >> field_name;
    field_id_t field_id = db.find_field(table_id, field_name);
    if (!field_id)
     std::cout << "Error: no such field: " << field_name << '\n';
    else
    {
     db.drop_field(table_id, field_id);
     std::cout << "OK: dropped field " << field_name << '\n';
    }
   }
  }
  else if (command == "insert_into") /////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(db, iss);
   if (table_id)
   {
    record_id_t record_id = 0;
    iss >> record_id;
    if (!db.insert_into(table_id, record_id))
     std::cout << "Error: could not insert with id: " << record_id << '\n';
    else
    {
     std::cout << "OK: inserted record, id = " << record_id << '\n';
     for (const auto &field:
          db.get_tables().find(table_id)->second.get_fields())
     {
      joedb::Value value = parse_value(field.second.type.get_type_id(), iss);
      db.update(table_id, record_id, field.first, value);
     }
    }
   }
  }
  else if (command == "update") //////////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(db, iss);
   if (table_id)
   {
    record_id_t record_id = 0;
    iss >> record_id;

    std::string field_name;
    iss >> field_name;
    field_id_t field_id = db.find_field(table_id, field_name);

    if (!field_id)
     std::cout << "Error: no such field: " << field_name << '\n';
    else
    {
     joedb::Value value = parse_value(db.get_field_type(table_id, field_id),
                                      iss);
     if (db.update(table_id, record_id, field_id, value))
      std::cout << "OK: updated\n";
     else
      std::cout << "Error: could not find record: " << record_id << '\n';
    }
   }
  }
  else if (command == "delete") //////////////////////////////////////////////
  {
   const table_id_t table_id = parse_table(db, iss);
   if (table_id)
   {
    record_id_t record_id = 0;
    iss >> record_id;
    if (db.delete_record(table_id, record_id))
     std::cout << "OK: record number " << record_id << " deleted\n";
    else
     std::cout << "Error: could not delete record " << record_id << '\n';
   }
  }
  else
   std::cout << "Error: unknown command: " << command << '\n';
 }

 return 0;
}
