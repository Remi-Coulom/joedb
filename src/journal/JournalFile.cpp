#include "JournalFile.h"
#include "File.h"
#include "Database.h"

using namespace joedb;

const uint32_t JournalFile::version_number = 0x00000001;
const int64_t JournalFile::header_size = 41;

/////////////////////////////////////////////////////////////////////////////
JournalFile::JournalFile(File &file):
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
   int64_t pos[4];
   for (int i = 0; i < 4; i++)
    pos[i] = file.read<int64_t>();

   if (pos[0] != pos[1] || pos[2] != pos[3])
    state = state_t::crash_check;

   int64_t position = 0;

   for (int i = 0; i < 2; i++)
    if (pos[2 * i] == pos[2 * i + 1] && pos[2 * i] > position)
    {
     position = pos[2 * i];
     checkpoint_index = i;
    }

   if (position < header_size)
    state = state_t::bad_format;
   else
    file.set_position(position);
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
void JournalFile::checkpoint()
{
 checkpoint_index ^= 1;
 const int64_t position = file.get_position();
 file.set_position(9 + 16 * checkpoint_index);
 file.write<int64_t>(position);
 file.write<int64_t>(position);
 file.set_position(position);
 file.flush();
}

/////////////////////////////////////////////////////////////////////////////
void JournalFile::replay_log(Database &db)
{
 file.set_position(header_size);

 while(true)
 {
  switch(file.read<operation_t>())
  {
   case operation_t::end_of_file:
   return;

   case operation_t::create_table:
    db.create_table(file.read_string());
   break;

   case operation_t::drop_table:
    db.drop_table(file.read<table_id_t>());
   break;

   case operation_t::add_field:
    db.add_field(file.read<table_id_t>(), file.read_string(), read_type());
   break;

   case operation_t::drop_field:
    db.drop_field(file.read<table_id_t>(), file.read<field_id_t>());
   break;

   case operation_t::insert_into:
    db.insert_into(file.read<table_id_t>(), file.read<record_id_t>());
   break;

   case operation_t::delete_from:
    db.delete_from(file.read<table_id_t>(), file.read<record_id_t>());
   break;

   case operation_t::update:
   {
   }
   break;
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
void JournalFile::after_create_table(const std::string &name)
{
 file.write<operation_t>(operation_t::create_table);
 file.write_string(name);
}

/////////////////////////////////////////////////////////////////////////////
void JournalFile::after_drop_table(table_id_t table_id)
{
 file.write<operation_t>(operation_t::drop_table);
 file.write<table_id_t>(table_id);
}

/////////////////////////////////////////////////////////////////////////////
void JournalFile::after_add_field(table_id_t table_id,
                                  const std::string &name,
                                  Type type)
{
 file.write<operation_t>(operation_t::add_field);
 file.write<table_id_t>(table_id);
 file.write_string(name);
 file.write<Type::type_id_t>(type.get_type_id());
 if (type.get_type_id() == Type::type_id_t::reference)
  file.write<table_id_t>(type.get_table_id());
}

/////////////////////////////////////////////////////////////////////////////
void JournalFile::after_drop_field(table_id_t table_id,
                                   field_id_t field_id)
{
 file.write<operation_t>(operation_t::drop_field);
 file.write<table_id_t>(table_id);
 file.write<field_id_t>(field_id);
}

/////////////////////////////////////////////////////////////////////////////
void JournalFile::after_insert(table_id_t table_id, record_id_t record_id)
{
 file.write<operation_t>(operation_t::insert_into);
 file.write<table_id_t>(table_id);
 file.write<record_id_t>(record_id);
}

/////////////////////////////////////////////////////////////////////////////
void JournalFile::after_delete(table_id_t table_id, record_id_t record_id)
{
 file.write<operation_t>(operation_t::delete_from);
 file.write<table_id_t>(table_id);
 file.write<record_id_t>(record_id);
}

/////////////////////////////////////////////////////////////////////////////
void JournalFile::after_update(table_id_t table_id,
                               record_id_t record_id,
                               field_id_t field_id,
                               const Value &value)
{
 file.write<operation_t>(operation_t::update);
 file.write<table_id_t>(table_id);
 file.write<record_id_t>(record_id);
 file.write<field_id_t>(field_id);

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
   file.write<record_id_t>(value.get_record_id());
  break;
 }
}

/////////////////////////////////////////////////////////////////////////////
Type JournalFile::read_type()
{
 Type::type_id_t type_id = file.read<Type::type_id_t>();
 if (type_id == Type::type_id_t::reference)
  return Type::reference(file.read<table_id_t>());
 else
  return Type(type_id);
}

/////////////////////////////////////////////////////////////////////////////
JournalFile::~JournalFile()
{
 checkpoint();
}
