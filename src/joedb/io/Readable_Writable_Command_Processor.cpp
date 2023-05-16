#include "joedb/io/Readable_Writable_Command_Processor.h"
#include "joedb/io/type_io.h"
#include "joedb/Exception.h"
#include "joedb/Readable.h"
#include "joedb/Writable.h"

#include <ctime>
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
   out << "Journal\n";
   out << "~~~~~~~\n";
   out << " timestamp [<stamp>] (if no value is given, use current time)\n";
   out << " comment \"<comment_string>\"\n";
   out << " valid_data\n";
   out << " checkpoint\n";
   out << " blob <data_string>\n";
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
   writable.create_table(table_name);
  }
  else if (command == "drop_table") /////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   writable.drop_table(table_id);
  }
  else if (command == "rename_table") ///////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   std::string new_name;
   iss >> new_name;
   writable.rename_table(table_id, new_name);
  }
  else if (command == "add_field") //////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   std::string field_name;
   iss >> field_name;
   const Type type = parse_type(iss, out);
   if (type.get_type_id() != Type::Type_Id::null)
    writable.add_field(table_id, field_name, type);
  }
  else if (command == "drop_field") ////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   std::string field_name;
   iss >> field_name;
   const Field_Id field_id = readable.find_field(table_id, field_name);
   writable.drop_field(table_id, field_id);
  }
  else if (command == "rename_field") //////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   std::string field_name;
   iss >> field_name;
   const Field_Id field_id = readable.find_field(table_id, field_name);
   std::string new_field_name;
   iss >> new_field_name;
   writable.rename_field(table_id, field_id, new_field_name);
  }
  else if (command == "custom") ////////////////////////////////////////////
  {
   std::string name;
   iss >> name;
   writable.custom(name);
  }
  else if (command == "comment") ///////////////////////////////////////////
  {
   const std::string comment = joedb::read_string(iss);
   writable.comment(comment);
  }
  else if (command == "timestamp") /////////////////////////////////////////
  {
   int64_t timestamp = 0;
   iss >> timestamp;
   if (iss.fail())
    timestamp = std::time(nullptr);
   writable.timestamp(timestamp);
  }
  else if (command == "valid_data") ////////////////////////////////////////
  {
   writable.valid_data();
  }
  else if (command == "checkpoint") ////////////////////////////////////////
  {
   writable.checkpoint(Commit_Level::no_commit);
  }
  else if (command == "insert_into") ///////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);

   Record_Id record_id = 0;
   iss >> record_id;
   if (record_id == 0)
    record_id = readable.get_last_record_id(table_id) + 1;

   writable.insert_into(table_id, record_id);
   if (iss.good())
    for (const auto &field: readable.get_fields(table_id))
    {
     update_value(iss, table_id, record_id, field.first);
     if (iss.fail())
      throw Exception("failed parsing value");
    }
  }
  else if (command == "insert_vector") /////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   Record_Id record_id = 0;
   Record_Id size = 0;
   iss >> record_id >> size;
   writable.insert_vector(table_id, record_id, size);
  }
  else if (command == "update") ////////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   Record_Id record_id = 0;
   iss >> record_id;
   std::string field_name;
   iss >> field_name;
   const Field_Id field_id = readable.find_field(table_id, field_name);
   update_value(iss, table_id, record_id, field_id);
  }
  else if (command == "update_vector") /////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   Record_Id record_id = 0;
   iss >> record_id;
   std::string field_name;
   iss >> field_name;
   const Field_Id field_id = readable.find_field(table_id, field_name);
   Record_Id size = 0;
   iss >> size;

   if (max_record_id != 0 && size >= max_record_id)
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
       v[i] = joedb::read_##type_id(iss);\
      writable.update_vector_##type_id(table_id, record_id, field_id, size, &v[0]);\
     }\
     break;
     #include "joedb/TYPE_MACRO.h"
    }
   }
  }
  else if (command == "delete_from") ////////////////////////////////////////
  {
   const Table_Id table_id = parse_table(iss, out);
   Record_Id record_id = 0;
   iss >> record_id;
   writable.delete_from(table_id, record_id);
  }
  else if (command == "blob") ///////////////////////////////////////////////
  {
   const std::string value = joedb::read_string(iss);

   const Blob blob = blob_writer ?
    blob_writer->write_blob_data(value) :
    writable.write_blob_data(value);

   joedb::write_blob(out, blob);
   out << '\n';
  }
  else
   return Status::not_found;

  return Status::done;
 }

 ////////////////////////////////////////////////////////////////////////////
 Readable_Writable_Command_Processor::~Readable_Writable_Command_Processor()
 ////////////////////////////////////////////////////////////////////////////
 {
  writable.checkpoint(Commit_Level::no_commit);
  if (blob_writer && blob_writer != &writable)
   blob_writer->checkpoint(Commit_Level::no_commit);
 }
}
