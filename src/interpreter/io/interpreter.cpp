#include "Database.h"
#include "dump.h"

#include <iostream>
#include <sstream>

joedb::Type parse_type(const joedb::Database &db, std::istream &is)
{
 std::string type_name;
 is >> type_name;
 if (type_name == "string")
  return joedb::Type::string();
 if (type_name == "int32")
  return joedb::Type::int32();
 if (type_name == "int64")
  return joedb::Type::int64();
 if (type_name == "references")
 {
  std::string table_name;
  is >> table_name;
  table_id_t table_id = db.find_table(table_name);
  if (table_id)
   return joedb::Type::reference(table_id);
 }

 return joedb::Type();
}

table_id_t parse_table(const joedb::Database &db, std::istream &is)
{
 std::string table_name;
 is >> table_name;
 table_id_t table_id = db.find_table(table_name);
 if (!table_id)
  std::cout << "Error: no such table: " << table_name << '\n';
 return table_id;
}

int main()
{
 joedb::Database db;
 std::string line;

 while(std::getline(std::cin, line))
 {
  std::istringstream iss(line);
  std::string command;
  iss >> command;

  if (command == "dump")
   joedb::dump(std::cout, db);
  else if (command == "quit")
   break;
  else if (command == "create_table")
  {
   std::string table_name;
   iss >> table_name;
   table_id_t table_id = db.create_table(table_name);
   std::cout << "OK: create table " << table_name <<
                "; table_id = " << table_id << '\n';
  }
  else if (command == "add_field")
  {
   const table_id_t table_id = parse_table(db, iss);
   if (table_id)
   {
    std::string field_name;
    iss >> field_name;
    joedb::Type type = parse_type(db, iss);
    if (type.get_kind() == joedb::Type::null_id)
    {
     std::cout << "Error: could not parse type\n";
    }
    else
    {
     field_id_t field_id = db.add_field(table_id, field_name, type);
     std::cout << "OK: add_field " << field_name <<
                  "; field_id = " << field_id << '\n';
    }
   }
  }
  else if (command == "insert_into")
  {
   const table_id_t table_id = parse_table(db, iss);
   if (table_id)
   {
    record_id_t record_id = 0;
    iss >> record_id;
    auto &table = db.get_tables().find(table_id)->second;
    if (table.get_records().find(record_id) != table.get_records().end())
     std::cout << "Error: record already exists\n";
    else
    {
     record_id_t result = db.insert_into(table_id, record_id);
     std::cout << "OK: inserted record, id = " << result << '\n';
     if (result == record_id)
     {
      for (const auto &field: table.get_fields())
      {
       joedb::Value value;
       switch(field.second.type.get_kind())
       {
        case joedb::Type::string_id:
        {
         std::string s;
         iss >> s;
         value = joedb::Value(s);
        }
        break;

        case joedb::Type::reference_id:
        {
         record_id_t record_id = 0;
         iss >> record_id;
         value = joedb::Value(record_id);
        }
        break;

        default:
         std::cout << "Error: unknown field type for field: " <<
                      field.second.name << '\n';
       }

       db.update(table_id, record_id, field.first, value);
      }
     }
    }
   }
  }
  else
   std::cout << "Error: unknown command: " << command << '\n';
 }

 return 0;
}
