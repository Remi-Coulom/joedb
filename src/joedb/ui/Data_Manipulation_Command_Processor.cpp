#include "joedb/ui/Data_Manipulation_Command_Processor.h"
#include "joedb/ui/type_io.h"
#include "joedb/Readable.h"
#include "joedb/Writable.h"
#include "joedb/error/Exception.h"

#include <vector>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Data_Manipulation_Command_Processor::update_value
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
 Command_Processor::Status Data_Manipulation_Command_Processor::process_command
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::string &command,
  std::istream &parameters,
  std::istream &in,
  std::ostream &out
 )
 {
  const Status status = Readable_Command_Processor::process_command
  (
   command,
   parameters,
   in,
   out
  );

  if (status == Status::done)
   return status;
  else if (command == "help") ///////////////////////////////////////////////
  {
   out << R"RRR(Data manipulation
~~~~~~~~~~~~~~~~~
 insert_into <table_name> <record_id>
 delete_from <table_name> <record_id>
 insert_vector <table_name> <record_id> <size>
 delete_vector <table_name> <record_id> <size>
 update <table_name> <record_id> <field_name> <value>
 update_vector <table_name> <record_id> <field_name> <N> <v_1> ... <v_N>

)RRR";

   return Status::ok;
  }
  else if (command == "insert_into") ///////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, readable);
   Record_Id record_id = Record_Id::null;
   parameters >> record_id;

   if (record_id.is_null())
    record_id = readable.get_size(table_id);

   writable.insert_into(table_id, record_id);

   if (parameters.good())
   {
    for (const auto &[fid, fname]: readable.get_fields(table_id))
    {
     update_value(parameters, table_id, record_id, fid);
     if (parameters.fail())
      throw Exception("failed parsing value");
    }
   }
  }
  else if (command == "delete_from") ////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, readable);
   Record_Id record_id = Record_Id::null;
   parameters >> record_id;

   writable.delete_from(table_id, record_id);
  }
  else if (command == "insert_vector") /////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, readable);
   Record_Id record_id = Record_Id::null;
   size_t size = 0;
   parameters >> record_id >> size;
   writable.insert_vector(table_id, record_id, size);
  }
  else if (command == "delete_vector") /////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, readable);
   Record_Id record_id = Record_Id::null;
   size_t size = 0;
   parameters >> record_id >> size;
   writable.delete_vector(table_id, record_id, size);
  }
  else if (command == "update") ////////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, readable);
   Record_Id record_id = Record_Id::null;
   parameters >> record_id;
   std::string field_name;
   parameters >> field_name;
   const Field_Id field_id = readable.find_field(table_id, field_name);
   update_value(parameters, table_id, record_id, field_id);
  }
  else if (command == "update_vector") /////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, readable);
   Record_Id record_id = Record_Id::null;
   parameters >> record_id;
   std::string field_name;
   parameters >> field_name;
   const Field_Id field_id = readable.find_field(table_id, field_name);
   size_t size = 0;
   parameters >> size;

   if (max_record_id.is_not_null() && size >= size_t(index_t(max_record_id)))
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
       v[i] = joedb::read_##type_id(parameters);\
      writable.update_vector_##type_id(table_id, record_id, field_id, size, &v[0]);\
     }\
     break;
     #include "joedb/TYPE_MACRO.h"
    }
   }
  }
  else
   return Status::not_found;

  return Status::done;
 }
}
