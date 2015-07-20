#include "Journal_File.h"
#include "File.h"
#include "Database.h"

const uint32_t joedb::Journal_File::version_number = 0x00000001;
const int64_t joedb::Journal_File::header_size = 41;

/////////////////////////////////////////////////////////////////////////////
joedb::Journal_File::Journal_File(File &file):
 file(file),
 checkpoint_index(0),
 state(state_t::no_error),
 table_of_last_operation(0),
 record_of_last_operation(0)
{
 if (!file.is_good())
 {
  state = state_t::bad_file;
  return;
 }

 //
 // Create a new empty joedb file
 //
 if (file.get_mode() == File::mode_t::create_new)
 {
  file.write<uint8_t>('j');
  file.write<uint8_t>('o');
  file.write<uint8_t>('e');
  file.write<uint8_t>('d');
  file.write<uint8_t>('b');
  file.write<uint32_t>(version_number);
  file.write<int64_t>(0);
  file.write<int64_t>(0);
  file.write<int64_t>(0);
  file.write<int64_t>(0);
  checkpoint();
 }

 //
 // Check the format of an existing joedb file
 //
 else
 {
  //
  // First, check for initial "joedb"
  //
  if (file.read<uint8_t>() != 'j' ||
      file.read<uint8_t>() != 'o' ||
      file.read<uint8_t>() != 'e' ||
      file.read<uint8_t>() != 'd' ||
      file.read<uint8_t>() != 'b')
  {
   state = state_t::bad_format;
  }
  else
  {
   //
   // Check version number
   //
   const uint32_t version = file.read<uint32_t>();
   if (version > version_number)
    state = state_t::unsupported_version;

   //
   // Find the most recent checkpoint
   //
   uint64_t pos[4];
   for (int i = 0; i < 4; i++)
    pos[i] = file.read<uint64_t>();

   if (pos[0] != pos[1] || pos[2] != pos[3])
    state = state_t::crash_check;

   checkpoint_position = 0;

   for (unsigned i = 0; i < 2; i++)
    if (pos[2 * i] == pos[2 * i + 1] && pos[2 * i] > checkpoint_position)
    {
     checkpoint_position = pos[2 * i];
     checkpoint_index = i;
    }

   if (checkpoint_position < header_size)
    state = state_t::bad_format;
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::checkpoint()
{
 if (state == state_t::no_error)
 {
  file.flush();
  checkpoint_index ^= 1;
  checkpoint_position = file.get_position();
  file.set_position(9 + 16 * checkpoint_index);
  file.write<uint64_t>(checkpoint_position);
  file.write<uint64_t>(checkpoint_position);
  file.set_position(checkpoint_position);
  file.flush();
 }
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::replay_log(Listener &listener)
{
 file.set_position(header_size);
 Database db_schema;

 while(file.get_position() < checkpoint_position &&
       state == state_t::no_error &&
       listener.is_good() &&
       !file.is_end_of_file())
 {
  switch(file.read<operation_t>())
  {
   case operation_t::end_of_file:
   return;

   case operation_t::create_table:
   {
    std::string name = file.read_string();
    db_schema.create_table(name);
    listener.after_create_table(name);
   }
   break;

   case operation_t::drop_table:
   {
    table_id_t table_id = file.compact_read<table_id_t>();
    db_schema.drop_table(table_id);
    listener.after_drop_table(table_id);
   }
   break;

   case operation_t::add_field:
   {
    table_id_t table_id = file.compact_read<table_id_t>();
    std::string name = file.read_string();
    Type type = read_type();
    db_schema.add_field(table_id, name, type);
    listener.after_add_field(table_id, name, type);
   }
   break;

   case operation_t::drop_field:
   {
    table_id_t table_id = file.compact_read<table_id_t>();
    field_id_t field_id = file.compact_read<field_id_t>();
    db_schema.drop_field(table_id, field_id);
    listener.after_drop_field(table_id, field_id);
   }
   break;

   case operation_t::insert_into:
   {
    table_id_t table_id = file.compact_read<table_id_t>();
    record_id_t record_id = file.compact_read<record_id_t>();
    listener.after_insert(table_id, record_id);
    table_of_last_operation = table_id;
    record_of_last_operation = record_id;
   }
   break;

   case operation_t::append:
    listener.after_insert(table_of_last_operation,
                          ++record_of_last_operation);
   break;

   case operation_t::delete_from:
   {
    table_id_t table_id = file.compact_read<table_id_t>();
    record_id_t record_id = file.compact_read<record_id_t>();
    listener.after_delete(table_id, record_id);
   }
   break;

   case operation_t::update:
    table_of_last_operation = file.compact_read<table_id_t>();
    record_of_last_operation = file.compact_read<record_id_t>();

   case operation_t::update_last:
   {
    field_id_t field_id = file.compact_read<field_id_t>();

    switch (db_schema.get_field_type(table_of_last_operation, field_id))
    {
     case Type::type_id_t::null:
     break;

#define UPDATE_CASE(cpp_type, type_id, read_method)\
     case Type::type_id_t::type_id:\
     {\
      cpp_type value = file.read_method();\
      listener.after_update_##type_id(table_of_last_operation,\
                                      record_of_last_operation,\
                                      field_id, value);\
     }\
     break;

     UPDATE_CASE(std::string, string, read_string)
     UPDATE_CASE(int32_t, int32, read<int32_t>)
     UPDATE_CASE(int64_t, int64, read<int64_t>)
     UPDATE_CASE(record_id_t, reference, compact_read<record_id_t>)

#undef UPDATE_CASE
    }

   }
   break;

   default:
    state = state_t::bad_format;
   break;
  }
 }

 if (file.get_position() != checkpoint_position)
  state = state_t::crash_check;
 else
  file.set_position(checkpoint_position); // get ready for writing
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_create_table(const std::string &name)
{
 file.write<operation_t>(operation_t::create_table);
 file.write_string(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_drop_table(table_id_t table_id)
{
 file.write<operation_t>(operation_t::drop_table);
 file.compact_write<table_id_t>(table_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_add_field(table_id_t table_id,
                                         const std::string &name,
                                         Type type)
{
 file.write<operation_t>(operation_t::add_field);
 file.compact_write<table_id_t>(table_id);
 file.write_string(name);
 file.write<Type::type_id_t>(type.get_type_id());
 if (type.get_type_id() == Type::type_id_t::reference)
  file.compact_write<table_id_t>(type.get_table_id());
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_drop_field(table_id_t table_id,
                                          field_id_t field_id)
{
 file.write<operation_t>(operation_t::drop_field);
 file.compact_write<table_id_t>(table_id);
 file.compact_write<field_id_t>(field_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_insert(table_id_t table_id,
                                      record_id_t record_id)
{
 if (table_id == table_of_last_operation &&
     record_id == record_of_last_operation + 1)
 {
  file.write<operation_t>(operation_t::append);
 }
 else
 {
  file.write<operation_t>(operation_t::insert_into);
  file.compact_write<table_id_t>(table_id);
  file.compact_write<record_id_t>(record_id);
 }

 table_of_last_operation = table_id;
 record_of_last_operation = record_id;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_delete(table_id_t table_id,
                                      record_id_t record_id)
{
 file.write<operation_t>(operation_t::delete_from);
 file.compact_write<table_id_t>(table_id);
 file.compact_write<record_id_t>(record_id);
}

/////////////////////////////////////////////////////////////////////////////
#define AFTER_UPDATE(return_type, type_id, write_method)\
void joedb::Journal_File::after_update_##type_id(table_id_t table_id,\
                                                record_id_t record_id,\
                                                field_id_t field_id,\
                                                return_type value)\
{\
 if (table_id == table_of_last_operation &&\
     record_id == record_of_last_operation)\
 {\
  file.write<operation_t>(operation_t::update_last);\
 }\
 else\
 {\
  file.write<operation_t>(operation_t::update);\
  file.compact_write<table_id_t>(table_id);\
  file.compact_write<record_id_t>(record_id);\
  table_of_last_operation = table_id;\
  record_of_last_operation = record_id;\
 }\
 file.compact_write<field_id_t>(field_id);\
 file.write_method(value);\
}

AFTER_UPDATE(const std::string &, string, write_string)
AFTER_UPDATE(int32_t, int32, write<int32_t>)
AFTER_UPDATE(int64_t, int64, write<int64_t>)
AFTER_UPDATE(record_id_t, reference, compact_write<record_id_t>)

#undef AFTER_UPDATE

/////////////////////////////////////////////////////////////////////////////
joedb::Type joedb::Journal_File::read_type()
{
 Type::type_id_t type_id = file.read<Type::type_id_t>();
 if (type_id == Type::type_id_t::reference)
  return Type::reference(file.compact_read<table_id_t>());
 else
  return Type(type_id);
}

/////////////////////////////////////////////////////////////////////////////
joedb::Journal_File::~Journal_File()
{
 checkpoint();
}
