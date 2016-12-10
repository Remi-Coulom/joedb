#include "joedb/Journal_File.h"
#include "joedb/Generic_File.h"

#include <vector>
#include <stdexcept>

const uint32_t joedb::Journal_File::version_number = 0x00000004;
const uint32_t joedb::Journal_File::compatible_version = 0x00000004;
const uint64_t joedb::Journal_File::header_size = 41;

/////////////////////////////////////////////////////////////////////////////
joedb::Journal_File::Journal_File(Generic_File &file):
/////////////////////////////////////////////////////////////////////////////
 file(file),
 checkpoint_index(0),
 checkpoint_position(0),
 current_commit_level(0),
 state(state_t::no_error),
 table_of_last_operation(0),
 record_of_last_operation(0),
 field_of_last_update(0)
{
 if (file.get_status() != Generic_File::status_t::success)
 {
  state = state_t::bad_file;
  return;
 }

 //
 // Create a new empty joedb file
 //
 if (file.get_mode() == Generic_File::mode_t::create_new)
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
  checkpoint(0);
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
   if (version < compatible_version || version > version_number)
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

   //
   // Compare to file size (if available)
   //
   int64_t file_size = file.get_size();
   if (file_size > 0 && uint64_t(file_size) != checkpoint_position)
    state = state_t::crash_check;
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
uint64_t joedb::Journal_File::ahead_of_checkpoint() const
/////////////////////////////////////////////////////////////////////////////
{
 return file.get_position() - checkpoint_position;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::checkpoint(int commit_level)
/////////////////////////////////////////////////////////////////////////////
{
 if ((ahead_of_checkpoint() || commit_level > current_commit_level) &&
     state == state_t::no_error)
 {
  checkpoint_index ^= 1;
  checkpoint_position = file.get_position();
  current_commit_level = commit_level;

  file.set_position(9 + 16 * checkpoint_index);
  file.write<uint64_t>(checkpoint_position);

  file.flush();
  if (commit_level > 0)
   file.commit();

  file.write<uint64_t>(checkpoint_position);

  file.flush();
  if (commit_level > 1)
   file.commit();

  file.set_position(checkpoint_position);
 }
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::replay_log(Listener &listener)
/////////////////////////////////////////////////////////////////////////////
{
 rewind();
 play_until(listener, checkpoint_position);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::rewind()
/////////////////////////////////////////////////////////////////////////////
{
 file.set_position(header_size);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::play_until(Listener &listener, uint64_t end)
/////////////////////////////////////////////////////////////////////////////
{
 if (end == 0)
  end = checkpoint_position;

 try
 {
  while(file.get_position() < end)
  {
   switch(file.read<operation_t>())
   {
    case operation_t::end_of_file:
     if (file.get_position() != checkpoint_position)
      state = state_t::crash_check;
    return;

    case operation_t::create_table:
    {
     std::string name = safe_read_string();
     listener.after_create_table(name);
    }
    break;

    case operation_t::drop_table:
    {
     table_id_t table_id = file.compact_read<table_id_t>();
     listener.after_drop_table(table_id);
    }
    break;

    case operation_t::rename_table:
    {
     table_id_t table_id = file.compact_read<table_id_t>();
     std::string name = safe_read_string();
     listener.after_rename_table(table_id, name);
    }
    break;

    case operation_t::add_field:
    {
     table_id_t table_id = file.compact_read<table_id_t>();
     std::string name = safe_read_string();
     Type type = read_type();
     listener.after_add_field(table_id, name, type);
    }
    break;

    case operation_t::drop_field:
    {
     table_id_t table_id = file.compact_read<table_id_t>();
     field_id_t field_id = file.compact_read<field_id_t>();
     listener.after_drop_field(table_id, field_id);
    }
    break;

    case operation_t::rename_field:
    {
     table_id_t table_id = file.compact_read<table_id_t>();
     field_id_t field_id = file.compact_read<field_id_t>();
     std::string name = safe_read_string();
     listener.after_rename_field(table_id, field_id, name);
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

    case operation_t::insert_vector:
    {
     table_id_t table_id = file.compact_read<table_id_t>();
     record_id_t record_id = file.compact_read<record_id_t>();
     record_id_t size = file.compact_read<record_id_t>();
     listener.after_insert_vector(table_id, record_id, size);
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

    #define TYPE_MACRO(cpp_type, return_type, type_id, read_method, W)\
    case operation_t::update_##type_id:\
     table_of_last_operation = file.compact_read<table_id_t>();\
     record_of_last_operation = file.compact_read<record_id_t>();\
     field_of_last_update = file.compact_read<field_id_t>();\
    goto lbl_perform_update_##type_id;\
\
    case operation_t::update_last_##type_id:\
     field_of_last_update = file.compact_read<field_id_t>();\
    goto lbl_perform_update_##type_id;\
\
    case operation_t::update_next_##type_id:\
     record_of_last_operation++;\
    goto lbl_perform_update_##type_id;\
\
    lbl_perform_update_##type_id:\
    {\
     cpp_type value = read_method();\
     listener.after_update_##type_id(table_of_last_operation,\
                                     record_of_last_operation,\
                                     field_of_last_update,\
                                     value);\
    }\
    break;\
\
    case operation_t::update_vector_##type_id:\
    {\
     table_of_last_operation = file.compact_read<table_id_t>();\
     record_of_last_operation = file.compact_read<record_id_t>();\
     field_of_last_update = file.compact_read<field_id_t>();\
     record_id_t size = file.compact_read<record_id_t>();\
     std::vector<cpp_type> buffer(size);\
     for (size_t i = 0; i < size; i++)\
      buffer[i] = read_method();\
     listener.after_update_vector_##type_id(table_of_last_operation,\
                                            record_of_last_operation,\
                                            field_of_last_update,\
                                            size,\
                                            &buffer[0]);\
    }\
    break;
    #include "joedb/TYPE_MACRO.h"
    #undef TYPE_MACRO

    case operation_t::custom:
    {
     std::string name = safe_read_string();
     listener.after_custom(name);
    }
    break;

    case operation_t::comment:
    {
     std::string comment = safe_read_string();
     listener.after_comment(comment);
    }
    break;

    case operation_t::timestamp:
    {
     int64_t timestamp = file.read<int64_t>();
     listener.after_timestamp(timestamp);
    }
    break;

    case operation_t::valid_data:
     listener.after_valid_data();
    break;

    default:
     state = state_t::bad_format;
    return;
   }
  }
 }
 catch (std::runtime_error e)
 {
  state = state_t::listener_threw;
  return;
 }

 if (file.get_position() != end)
 {
  state = state_t::crash_check;
  return;
 }

 file.set_position(end); // get ready for writing
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_create_table(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::create_table);
 file.write_string(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_drop_table(table_id_t table_id)
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::drop_table);
 file.compact_write<table_id_t>(table_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_rename_table
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name
)
{
 file.write<operation_t>(operation_t::rename_table);
 file.compact_write<table_id_t>(table_id);
 file.write_string(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_add_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name,
 Type type
)
{
 file.write<operation_t>(operation_t::add_field);
 file.compact_write<table_id_t>(table_id);
 file.write_string(name);
 file.write<Type::type_id_t>(type.get_type_id());
 if (type.get_type_id() == Type::type_id_t::reference)
  file.compact_write<table_id_t>(type.get_table_id());
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_drop_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 field_id_t field_id
)
{
 file.write<operation_t>(operation_t::drop_field);
 file.compact_write<table_id_t>(table_id);
 file.compact_write<field_id_t>(field_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_rename_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 field_id_t field_id,
 const std::string &name
)
{
 file.write<operation_t>(operation_t::rename_field);
 file.compact_write<table_id_t>(table_id);
 file.compact_write<field_id_t>(field_id);
 file.write_string(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_custom(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::custom);
 file.write_string(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_comment(const std::string &comment)
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::comment);
 file.write_string(comment);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_timestamp(int64_t timestamp)
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::timestamp);
 file.write<int64_t>(timestamp);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_valid_data()
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::valid_data);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_insert
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id
)
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
void joedb::Journal_File::after_insert_vector
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id,
 record_id_t size
)
{
 file.write<operation_t>(operation_t::insert_vector);
 file.compact_write<table_id_t>(table_id);
 file.compact_write<record_id_t>(record_id);
 file.compact_write<record_id_t>(size);

 table_of_last_operation = table_id;
 record_of_last_operation = record_id;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::after_delete
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id
)
{
 file.write<operation_t>(operation_t::delete_from);
 file.compact_write<table_id_t>(table_id);
 file.compact_write<record_id_t>(record_id);
}

/////////////////////////////////////////////////////////////////////////////
#define TYPE_MACRO(type, return_type, type_id, R, write_method)\
void joedb::Journal_File::after_update_##type_id\
(\
 table_id_t table_id,\
 record_id_t record_id,\
 field_id_t field_id,\
 return_type value\
)\
{\
 if (table_id == table_of_last_operation &&\
     record_id == record_of_last_operation)\
 {\
  file.write<operation_t>(operation_t::update_last_##type_id);\
  file.compact_write<field_id_t>(field_id);\
  field_of_last_update = field_id;\
 }\
 else if (table_id == table_of_last_operation &&\
          record_id == record_of_last_operation + 1 &&\
          field_id == field_of_last_update)\
 {\
  file.write<operation_t>(operation_t::update_next_##type_id);\
  record_of_last_operation++;\
 }\
 else\
 {\
  file.write<operation_t>(operation_t::update_##type_id);\
  file.compact_write<table_id_t>(table_id);\
  file.compact_write<record_id_t>(record_id);\
  file.compact_write<field_id_t>(field_id);\
  table_of_last_operation = table_id;\
  record_of_last_operation = record_id;\
  field_of_last_update = field_id;\
 }\
 file.write_method(value);\
}\
void joedb::Journal_File::after_update_vector_##type_id\
(\
 table_id_t table_id,\
 record_id_t record_id,\
 field_id_t field_id,\
 record_id_t size,\
 const type *value\
)\
{\
 file.write<operation_t>(operation_t::update_vector_##type_id);\
 file.compact_write<table_id_t>(table_id);\
 file.compact_write<record_id_t>(record_id);\
 file.compact_write<field_id_t>(field_id);\
 file.compact_write<record_id_t>(size);\
 table_of_last_operation = table_id;\
 record_of_last_operation = record_id;\
 field_of_last_update = field_id;\
 for (record_id_t i = 0; i < size; i++)\
  file.write_method(value[i]);\
}
#include "joedb/TYPE_MACRO.h"
#undef TYPE_MACRO

/////////////////////////////////////////////////////////////////////////////
joedb::Type joedb::Journal_File::read_type()
/////////////////////////////////////////////////////////////////////////////
{
 Type::type_id_t type_id = file.read<Type::type_id_t>();
 if (type_id == Type::type_id_t::reference)
  return Type::reference(file.compact_read<table_id_t>());
 else
  return Type(type_id);
}

/////////////////////////////////////////////////////////////////////////////
std::string joedb::Journal_File::safe_read_string()
/////////////////////////////////////////////////////////////////////////////
{
 return file.safe_read_string(checkpoint_position);
}

/////////////////////////////////////////////////////////////////////////////
joedb::Journal_File::~Journal_File()
/////////////////////////////////////////////////////////////////////////////
{
 checkpoint(0);
}
