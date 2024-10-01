#include "joedb/io/Readable_Writable_Command_Processor.h"
#include "joedb/io/type_io.h"
#include "joedb/Readable.h"
#include "joedb/Writable.h"
#include "joedb/Exception.h"

#include <vector>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Type Readable_Writable_Command_Processor::parse_type
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
   const Table_Id table_id = get_readable().find_table(table_name);
   if (table_id != Table_Id(0))
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
  std::istream &parameters,
  std::istream &in,
  std::ostream &out
 )
 {
  if (command == "help") ////////////////////////////////////////////////////
  {
   out << R"RRR(Data definition
~~~~~~~~~~~~~~~
 create_table <table_name>
 drop_table <table_name>
 rename_table <old_table_name> <new_table_name>
 add_field <table_name> <field_name> <type> [default <value>]
 drop_field <table_name> <field_name>
 rename_field <table_name> <old_field_name> <new_field_name>
 custom <custom_name>

 <type> may be:
  string,
  blob,
  int8, int16, int32, int64,
  float32, float64,
  boolean,
  references <table_name>

Data manipulation
~~~~~~~~~~~~~~~~~
 insert_into <table_name> <record_id>
 insert_vector <table_name> <record_id> <size>
 update <table_name> <record_id> <field_name> <value>
 update_vector <table_name> <record_id> <field_name> <N> <v_1> ... <v_N>
 delete_from <table_name> <record_id>

)RRR";

   return Status::ok;
  }
  else if (command == "create_table") ///////////////////////////////////////
  {
   std::string table_name;
   parameters >> table_name;
   get_writable().create_table(table_name);
  }
  else if (command == "drop_table") /////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, out);
   get_writable().drop_table(table_id);
  }
  else if (command == "rename_table") ///////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, out);
   std::string new_name;
   parameters >> new_name;
   get_writable().rename_table(table_id, new_name);
  }
  else if (command == "add_field") //////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, out);
   std::string field_name;
   parameters >> field_name;
   const Type type = parse_type(parameters, out);
   if (type.get_type_id() != Type::Type_Id::null)
   {
    get_writable().add_field(table_id, field_name, type);

    std::string next_word;
    parameters >> next_word;

    if (next_word == "=")
    {
     const Field_Id field_id =
      get_readable().get_fields(table_id).rbegin()->first;

     const Record_Id last_record_id =
      get_readable().get_last_record_id(table_id);\

     switch(type.get_type_id())
     {
      case Type::Type_Id::null: break;
      #define TYPE_MACRO(type, return_type, type_id, read_method, write_method)\
      case Type::Type_Id::type_id:\
      {\
       const type value = joedb::read_##type_id(parameters);\
       for (Record_Id record_id = Record_Id(1); record_id <= last_record_id; ++record_id)\
        if (get_readable().is_used(table_id, record_id))\
         get_writable().update_##type_id(table_id, record_id, field_id, value);\
      }\
      break;
      #include "joedb/TYPE_MACRO.h"
     }
    }
   }
  }
  else if (command == "drop_field") ////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, out);
   std::string field_name;
   parameters >> field_name;
   const Field_Id field_id = get_readable().find_field(table_id, field_name);
   get_writable().drop_field(table_id, field_id);
  }
  else if (command == "rename_field") //////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, out);
   std::string field_name;
   parameters >> field_name;
   const Field_Id field_id = get_readable().find_field(table_id, field_name);
   std::string new_field_name;
   parameters >> new_field_name;
   get_writable().rename_field(table_id, field_id, new_field_name);
  }
  else if (command == "custom") ////////////////////////////////////////////
  {
   std::string name;
   parameters >> name;
   get_writable().custom(name);
  }
  else if (command == "insert_into") ///////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, out);

   Record_Id record_id = Record_Id(0);
   parameters >> record_id;
   if (record_id == Record_Id(0))
    record_id = get_readable().get_last_record_id(table_id) + 1;

   get_writable().insert_into(table_id, record_id);
   if (parameters.good())
    for (const auto &[fid, fname]: get_readable().get_fields(table_id))
    {
     update_value(parameters, table_id, record_id, fid);
     if (parameters.fail())
      throw Exception("failed parsing value");
    }
  }
  else if (command == "insert_vector") /////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, out);
   Record_Id record_id = Record_Id(0);
   size_t size = 0;
   parameters >> record_id >> size;
   get_writable().insert_vector(table_id, record_id, size);
  }
  else if (command == "update") ////////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, out);
   Record_Id record_id = Record_Id(0);
   parameters >> record_id;
   std::string field_name;
   parameters >> field_name;
   const Field_Id field_id = get_readable().find_field(table_id, field_name);
   update_value(parameters, table_id, record_id, field_id);
  }
  else if (command == "update_vector") /////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, out);
   Record_Id record_id = Record_Id(0);
   parameters >> record_id;
   std::string field_name;
   parameters >> field_name;
   const Field_Id field_id = get_readable().find_field(table_id, field_name);
   size_t size = 0;
   parameters >> size;

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
       v[i] = joedb::read_##type_id(parameters);\
      get_writable().update_vector_##type_id(table_id, record_id, field_id, size, &v[0]);\
     }\
     break;
     #include "joedb/TYPE_MACRO.h"
    }
   }
  }
  else if (command == "delete_from") ////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, out);
   Record_Id record_id = Record_Id(0);
   parameters >> record_id;
   get_writable().delete_from(table_id, record_id);
  }
  else
   return Status::not_found;

  return Status::done;
 }
}
