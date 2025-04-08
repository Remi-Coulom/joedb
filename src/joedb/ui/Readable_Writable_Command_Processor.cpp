#include "joedb/ui/Readable_Writable_Command_Processor.h"
#include "joedb/ui/type_io.h"
#include "joedb/Readable.h"
#include "joedb/Writable.h"
#include "joedb/error/Exception.h"

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
   const Table_Id table_id = readable.find_table(table_name);
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
 Command_Processor::Status Readable_Writable_Command_Processor::process_command
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::string &command,
  std::istream &parameters,
  std::istream &in,
  std::ostream &out
 )
 {
  const Status status = Data_Manipulation_Command_Processor::process_command
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

)RRR";

   return Status::ok;
  }
  else if (command == "create_table") ///////////////////////////////////////
  {
   std::string table_name;
   parameters >> table_name;
   writable.create_table(table_name);
  }
  else if (command == "drop_table") /////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, readable);
   writable.drop_table(table_id);
  }
  else if (command == "rename_table") ///////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, readable);
   std::string new_name;
   parameters >> new_name;
   writable.rename_table(table_id, new_name);
  }
  else if (command == "add_field") //////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, readable);
   std::string field_name;
   parameters >> field_name;
   const Type type = parse_type(parameters, out);
   if (type.get_type_id() != Type::Type_Id::null)
   {
    writable.add_field(table_id, field_name, type);

    std::string next_word;
    parameters >> next_word;

    if (next_word == "=")
    {
     const Field_Id field_id =
      readable.get_fields(table_id).rbegin()->first;

     const Record_Id last_record_id =
      readable.get_last_record_id(table_id);\

     switch(type.get_type_id())
     {
      case Type::Type_Id::null: break;
      #define TYPE_MACRO(type, return_type, type_id, read_method, write_method)\
      case Type::Type_Id::type_id:\
      {\
       const type value = joedb::read_##type_id(parameters);\
       for (Record_Id record_id = Record_Id(1); record_id <= last_record_id; ++record_id)\
        if (readable.is_used(table_id, record_id))\
         writable.update_##type_id(table_id, record_id, field_id, value);\
      }\
      break;
      #include "joedb/TYPE_MACRO.h"
     }
    }
   }
  }
  else if (command == "drop_field") ////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, readable);
   std::string field_name;
   parameters >> field_name;
   const Field_Id field_id = readable.find_field(table_id, field_name);
   writable.drop_field(table_id, field_id);
  }
  else if (command == "rename_field") //////////////////////////////////////
  {
   const Table_Id table_id = parse_table(parameters, readable);
   std::string field_name;
   parameters >> field_name;
   const Field_Id field_id = readable.find_field(table_id, field_name);
   std::string new_field_name;
   parameters >> new_field_name;
   writable.rename_field(table_id, field_id, new_field_name);
  }
  else if (command == "custom") ////////////////////////////////////////////
  {
   std::string name;
   parameters >> name;
   writable.custom(name);
  }
  else
   return Status::not_found;

  return Status::done;
 }
}
