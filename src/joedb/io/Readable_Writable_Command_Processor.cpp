#include "joedb/io/Readable_Writable_Command_Processor.h"
#include "joedb/io/type_io.h"
#include "joedb/Readable.h"
#include "joedb/Writable.h"

#include <vector>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Readable_Writable_Command_Processor::update_value
 ////////////////////////////////////////////////////////////////////////////
 (
  std::istream &in,
  Table_Id table_id,
  Record_Id record_id,
  Field_Id field_id
 )
 {
  switch(get_readable().get_field_type(table_id, field_id).get_type_id())
  {
   case Type::Type_Id::null:
    throw Exception("bad field");

   #define TYPE_MACRO(type, return_type, type_id, read_method, write_method)\
   case Type::Type_Id::type_id:\
   {\
    const type value = joedb::read_##type_id(in);\
    get_writable().update_##type_id(table_id, record_id, field_id, value);\
   }\
   break;
   #include "joedb/TYPE_MACRO.h"
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 Command_Processor::Status Readable_Writable_Command_Processor::process_command
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::string &command,
  std::istream &iss,
  std::ostream &out
 )
 {
  if (command == "help") ////////////////////////////////////////////////////
  {
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

   return Status::ok;
  }
  else if (command == "create_table") ///////////////////////////////////////
  {
   std::string table_name;
   iss >> table_name;
   get_writable().create_table(table_name);
  }
  else if (command == "drop_table") /////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   get_writable().drop_table(table_id);
  }
  else if (command == "rename_table") ///////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   std::string new_name;
   iss >> new_name;
   get_writable().rename_table(table_id, new_name);
  }
  else if (command == "add_field") //////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   std::string field_name;
   iss >> field_name;
   const Type type = parse_type(iss, out);
   if (type.get_type_id() != Type::Type_Id::null)
    get_writable().add_field(table_id, field_name, type);
  }
  else if (command == "drop_field") ////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   std::string field_name;
   iss >> field_name;
   const Field_Id field_id = get_readable().find_field(table_id, field_name);
   get_writable().drop_field(table_id, field_id);
  }
  else if (command == "rename_field") //////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   std::string field_name;
   iss >> field_name;
   const Field_Id field_id = get_readable().find_field(table_id, field_name);
   std::string new_field_name;
   iss >> new_field_name;
   get_writable().rename_field(table_id, field_id, new_field_name);
  }
  else if (command == "custom") ////////////////////////////////////////////
  {
   std::string name;
   iss >> name;
   get_writable().custom(name);
  }
  else if (command == "insert_into") ///////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);

   Record_Id record_id = Record_Id(0);
   iss >> record_id;
   if (record_id == Record_Id(0))
    record_id = get_readable().get_last_record_id(table_id) + 1;

   get_writable().insert_into(table_id, record_id);
   if (iss.good())
    for (const auto &field: get_readable().get_fields(table_id))
    {
     update_value(iss, table_id, record_id, field.first);
     if (iss.fail())
      throw Exception("failed parsing value");
    }
  }
  else if (command == "insert_vector") /////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   Record_Id record_id = Record_Id(0);
   Size size = 0;
   iss >> record_id >> size;
   get_writable().insert_vector(table_id, record_id, size);
  }
  else if (command == "update") ////////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   Record_Id record_id = Record_Id(0);
   iss >> record_id;
   std::string field_name;
   iss >> field_name;
   const Field_Id field_id = get_readable().find_field(table_id, field_name);
   update_value(iss, table_id, record_id, field_id);
  }
  else if (command == "update_vector") /////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   Record_Id record_id = Record_Id(0);
   iss >> record_id;
   std::string field_name;
   iss >> field_name;
   const Field_Id field_id = get_readable().find_field(table_id, field_name);
   Size size = 0;
   iss >> size;

   if (max_record_id != 0 && size >= max_record_id)
    throw Exception("vector is too big");
   else
   {
    switch(get_readable().get_field_type(table_id, field_id).get_type_id())
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
      get_writable().update_vector_##type_id(table_id, record_id, field_id, size, &v[0]);\
     }\
     break;
     #include "joedb/TYPE_MACRO.h"
    }
   }
  }
  else if (command == "delete_from") ////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   Record_Id record_id = Record_Id(0);
   iss >> record_id;
   get_writable().delete_from(table_id, record_id);
  }
  else
   return Status::not_found;

  return Status::done;
 }
}
