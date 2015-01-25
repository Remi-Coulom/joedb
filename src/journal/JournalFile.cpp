#include "JournalFile.h"
#include "File.h"
#include "Database.h"

const uint32_t joedb::JournalFile::version_number = 0x00000001;
const int64_t joedb::JournalFile::header_size = 41;

/////////////////////////////////////////////////////////////////////////////
joedb::JournalFile::JournalFile(File &file):
 file(file),
 checkpoint_index(0),
 state(state_t::no_error)
{
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
void joedb::JournalFile::checkpoint()
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
void joedb::JournalFile::replay_log(Listener &listener)
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
   }
   break;

   case operation_t::delete_from:
   {
    table_id_t table_id = file.compact_read<table_id_t>();
    record_id_t record_id = file.compact_read<record_id_t>();
    listener.after_delete(table_id, record_id);
   }
   break;

   case operation_t::update:
   {
    table_id_t table_id = file.compact_read<table_id_t>();
    record_id_t record_id = file.compact_read<record_id_t>();
    field_id_t field_id = file.compact_read<field_id_t>();

    Value value;
    switch (db_schema.get_field_type(table_id, field_id))
    {
     case Type::type_id_t::null:
     break;

     case Type::type_id_t::string:
      value = Value(file.read_string());
     break;

     case Type::type_id_t::int32:
      value = Value(file.read<int32_t>());
     break;

     case Type::type_id_t::int64:
      value = Value(file.read<int64_t>());
     break;

     case Type::type_id_t::reference:
      value = Value(file.compact_read<record_id_t>());
     break;
    }

    listener.after_update(table_id, record_id, field_id, value);
   }
   break;

   default:
    state = state_t::bad_format;
   break;
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
void joedb::JournalFile::after_create_table(const std::string &name)
{
 file.write<operation_t>(operation_t::create_table);
 file.write_string(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::JournalFile::after_drop_table(table_id_t table_id)
{
 file.write<operation_t>(operation_t::drop_table);
 file.compact_write<table_id_t>(table_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::JournalFile::after_add_field(table_id_t table_id,
                                  const std::string &name,
                                  Type type)
{
 file.write<operation_t>(operation_t::add_field);
 file.compact_write<table_id_t>(table_id);
 file.write_string(name);
 file.write<Type::type_id_t>(type.get_type_id());
 if (type.get_type_id() == Type::type_id_t::reference)
  file.write<table_id_t>(type.get_table_id());
}

/////////////////////////////////////////////////////////////////////////////
void joedb::JournalFile::after_drop_field(table_id_t table_id,
                                   field_id_t field_id)
{
 file.write<operation_t>(operation_t::drop_field);
 file.compact_write<table_id_t>(table_id);
 file.compact_write<field_id_t>(field_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::JournalFile::after_insert(table_id_t table_id, record_id_t record_id)
{
 file.write<operation_t>(operation_t::insert_into);
 file.compact_write<table_id_t>(table_id);
 file.compact_write<record_id_t>(record_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::JournalFile::after_delete(table_id_t table_id, record_id_t record_id)
{
 file.write<operation_t>(operation_t::delete_from);
 file.compact_write<table_id_t>(table_id);
 file.compact_write<record_id_t>(record_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::JournalFile::after_update(table_id_t table_id,
                               record_id_t record_id,
                               field_id_t field_id,
                               const Value &value)
{
 file.write<operation_t>(operation_t::update);
 file.compact_write<table_id_t>(table_id);
 file.compact_write<record_id_t>(record_id);
 file.compact_write<field_id_t>(field_id);

 switch(value.get_type_id())
 {
  case Type::type_id_t::null:
  break;

  case Type::type_id_t::string:
   file.write_string(value.get_string());
  break;

  case Type::type_id_t::int32:
   file.write<int32_t>(value.get_int32());
  break;

  case Type::type_id_t::int64:
   file.write<int64_t>(value.get_int64());
  break;

  case Type::type_id_t::reference:
   file.compact_write<record_id_t>(value.get_record_id());
  break;
 }
}

/////////////////////////////////////////////////////////////////////////////
joedb::Type joedb::JournalFile::read_type()
{
 Type::type_id_t type_id = file.read<Type::type_id_t>();
 if (type_id == Type::type_id_t::reference)
  return Type::reference(file.compact_read<table_id_t>());
 else
  return Type(type_id);
}

/////////////////////////////////////////////////////////////////////////////
joedb::JournalFile::~JournalFile()
{
 checkpoint();
}
