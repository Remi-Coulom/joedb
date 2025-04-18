#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/Buffered_File.h"
#include "joedb/error/Exception.h"

#include <vector>

#if __cplusplus < 201703L
constexpr uint32_t joedb::Readonly_Journal::version_number;
constexpr uint32_t joedb::Readonly_Journal::compatible_version;
constexpr int64_t joedb::Readonly_Journal::header_size;
#endif

/////////////////////////////////////////////////////////////////////////////
#define TYPE_MACRO(cpp_type, return_type, type_id, read_method, W)\
void joedb::Readonly_Journal::perform_update_##type_id(Writable &writable)\
{\
 const cpp_type value = read_method();\
 writable.update_##type_id\
 (\
  table_of_last_operation,\
  record_of_last_operation,\
  field_of_last_update,\
  value\
 );\
}
#include "joedb/TYPE_MACRO.h"

/////////////////////////////////////////////////////////////////////////////
joedb::Readonly_Journal::Readonly_Journal
/////////////////////////////////////////////////////////////////////////////
(
 Journal_Construction_Lock &lock,
 Check check
):
 file(lock.get_file()),
 file_version(0),
 checkpoint_index(0),
 checkpoint_position(header_size),
 table_of_last_operation(Table_Id(0)),
 record_of_last_operation(Record_Id(0)),
 field_of_last_update(Field_Id(0))
{
 file.set_position(0);

 //
 // Check the format of an existing joedb file
 //
 if (!lock.is_creating_new())
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
   if (check_flag(check, Check::joedb))
    throw Exception("File does not start by 'joedb'");
  }
  else
  {
   //
   // Check version number
   //
   file_version = file.read<uint32_t>();

   if (check_flag(check, Check::version))
   {
    if (file_version < compatible_version || file_version > version_number)
     throw Exception("Unsupported format version");
   }

   checkpoint_position = header_size;

   for (int i = 0; i < 4; i++)
    lock.pos[i] = file.read<int64_t>();

   read_checkpoint(lock.pos);

   //
   // Compare to file size (if available)
   //
   const int64_t file_size = file.get_size();

   if (file_size > 0)
   {
    if (check_flag(check, Check::set_checkpoint))
     checkpoint_position = file_size;
    else
    {
     if
     (
      check_flag(check, Check::big_size) &&
      file_size > checkpoint_position
     )
     {
      throw Exception
      (
       "Checkpoint (" + std::to_string(checkpoint_position) + ") is smaller than file size (" +
       std::to_string(file_size) + "). This file may contain an aborted transaction. "
       "'joedb_push file.joedb file fixed.joedb' can be used to truncate it."
      );
     }

     if
     (
      check_flag(check, Check::small_size) &&
      file_size < checkpoint_position
     )
     {
      throw Exception("Checkpoint is bigger than file size");
     }
    }
   }
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::read_checkpoint
/////////////////////////////////////////////////////////////////////////////
(
 const std::array<int64_t, 4> &pos
)
{
 for (unsigned i = 0; i < 2; i++)
 {
  if (pos[2 * i] == pos[2 * i + 1] && pos[2 * i] >= checkpoint_position)
  {
   checkpoint_position = pos[2 * i];
   checkpoint_index = i;
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::pull_without_locking()
/////////////////////////////////////////////////////////////////////////////
{
 const int64_t old_position = file.get_position();
 std::array<int64_t, 4> pos;
 file.pread((char *)&pos, sizeof(pos), checkpoint_offset);
 read_checkpoint(pos);
 file.set_position(old_position);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::pull()
/////////////////////////////////////////////////////////////////////////////
{
 file.shared_lock_head();
 pull_without_locking();
 file.unlock_head();
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::replay_log(Writable &writable)
/////////////////////////////////////////////////////////////////////////////
{
 rewind();
 play_until_checkpoint(writable);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::replay_with_checkpoint_comments
/////////////////////////////////////////////////////////////////////////////
(
 Writable &writable
)
{
 rewind();
 while(file.get_position() < checkpoint_position)
 {
  one_step(writable);
  writable.comment(std::to_string(file.get_position()));
 }
 writable.default_checkpoint();
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::rewind()
/////////////////////////////////////////////////////////////////////////////
{
 file.set_position(header_size);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::set_position(int64_t position)
/////////////////////////////////////////////////////////////////////////////
{
 file.set_position(position);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::play_until(Writable &writable, int64_t end)
/////////////////////////////////////////////////////////////////////////////
{
 while(file.get_position() < end)
  one_step(writable);
 file.set_position(file.get_position()); // get ready for writing
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Readonly_Journal::at_end_of_file() const
/////////////////////////////////////////////////////////////////////////////
{
 return file.get_position() >= checkpoint_position;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::one_step(Writable &writable)
/////////////////////////////////////////////////////////////////////////////
{
 switch(file.read<operation_t>())
 {
  case operation_t::create_table:
  {
   std::string name = safe_read_string();
   writable.create_table(name);
  }
  break;

  case operation_t::drop_table:
  {
   const Table_Id table_id = file.read_strong_type<Table_Id>();
   writable.drop_table(table_id);
  }
  break;

  case operation_t::rename_table:
  {
   const Table_Id table_id = file.read_strong_type<Table_Id>();
   std::string name = safe_read_string();
   writable.rename_table(table_id, name);
  }
  break;

  case operation_t::add_field:
  {
   const Table_Id table_id = file.read_strong_type<Table_Id>();
   std::string name = safe_read_string();
   const Type type = read_type();
   writable.add_field(table_id, name, type);
  }
  break;

  case operation_t::drop_field:
  {
   const Table_Id table_id = file.read_strong_type<Table_Id>();
   const Field_Id field_id = file.read_strong_type<Field_Id>();
   writable.drop_field(table_id, field_id);
  }
  break;

  case operation_t::rename_field:
  {
   const Table_Id table_id = file.read_strong_type<Table_Id>();
   const Field_Id field_id = file.read_strong_type<Field_Id>();
   std::string name = safe_read_string();
   writable.rename_field(table_id, field_id, name);
  }
  break;

  case operation_t::insert_into:
  {
   const Table_Id table_id = file.read_strong_type<Table_Id>();
   const Record_Id record_id = file.read_strong_type<Record_Id>();
   writable.insert_into(table_id, record_id);
   table_of_last_operation = table_id;
   record_of_last_operation = record_id;
  }
  break;

  case operation_t::insert_vector:
  {
   const Table_Id table_id = file.read_strong_type<Table_Id>();
   const Record_Id record_id = file.read_strong_type<Record_Id>();
   const size_t size = file.compact_read<size_t>();
   writable.insert_vector(table_id, record_id, size);
   table_of_last_operation = table_id;
   record_of_last_operation = record_id;
  }
  break;

  case operation_t::append:
   writable.insert_into(table_of_last_operation,
                         ++record_of_last_operation);
  break;

  case operation_t::delete_from:
  {
   const Table_Id table_id = file.read_strong_type<Table_Id>();
   const Record_Id record_id = file.read_strong_type<Record_Id>();
   writable.delete_from(table_id, record_id);
  }
  break;

  #define TYPE_MACRO(cpp_type, return_type, type_id, read_method, W)\
  case operation_t::update_##type_id:\
   table_of_last_operation = file.read_strong_type<Table_Id>();\
   record_of_last_operation = file.read_strong_type<Record_Id>();\
   field_of_last_update = file.read_strong_type<Field_Id>();\
   perform_update_##type_id(writable);\
  break;\
\
  case operation_t::update_last_##type_id:\
   field_of_last_update = file.read_strong_type<Field_Id>();\
   perform_update_##type_id(writable);\
  break;\
\
  case operation_t::update_next_##type_id:\
   ++record_of_last_operation;\
   perform_update_##type_id(writable);\
  break;
  #include "joedb/TYPE_MACRO.h"

  #define TYPE_MACRO(cpp_type, return_type, type_id, read_method, W)\
  case operation_t::update_vector_##type_id:\
  {\
   table_of_last_operation = file.read_strong_type<Table_Id>();\
   record_of_last_operation = file.read_strong_type<Record_Id>();\
   field_of_last_update = file.read_strong_type<Field_Id>();\
   const size_t size = file.compact_read<size_t>();\
   if (int64_t(size) > checkpoint_position)\
    throw Exception("update_vector too big");\
   size_t capacity;\
   cpp_type *data = writable.get_own_##type_id##_storage\
   (\
    table_of_last_operation,\
    record_of_last_operation,\
    field_of_last_update,\
    capacity\
   );\
   std::vector<cpp_type> buffer;\
   if (!data)\
   {\
    buffer.resize(size);\
    data = &buffer[0];\
   }\
   else if (to_underlying(record_of_last_operation) <= 0 || to_underlying(record_of_last_operation) + size - 1 > capacity)\
    throw Exception("update_vector out of range");\
   read_vector_of_##type_id(data, size);\
   writable.update_vector_##type_id\
   (\
    table_of_last_operation,\
    record_of_last_operation,\
    field_of_last_update,\
    size,\
    data\
   );\
  }\
  break;
  #include "joedb/TYPE_MACRO.h"

  case operation_t::custom:
  {
   const std::string name = safe_read_string();
   writable.custom(name);
  }
  break;

  case operation_t::comment:
  {
   const std::string comment = safe_read_string();
   writable.comment(comment);
  }
  break;

  case operation_t::timestamp:
  {
   const int64_t timestamp = file.read<int64_t>();
   writable.timestamp(timestamp);
  }
  break;

  case operation_t::valid_data:
   writable.valid_data();
  break;

  case operation_t::blob:
  {
   writable.on_blob(Blob(get_position()));

   if (writable.wants_blob_data())
    writable.write_blob_data(safe_read_string());
   else
   {
    const int64_t size = file.compact_read<int64_t>();
    file.ignore(size);
   }
  }
  break;

  default:
  {
   throw Exception("Unexpected operation: file.get_position() = " + std::to_string(file.get_position()));
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
joedb::Type joedb::Readonly_Journal::read_type()
/////////////////////////////////////////////////////////////////////////////
{
 const Type::Type_Id type_id = Type::Type_Id(file.read<Type_Id_Storage>());
 if (type_id == Type::Type_Id::reference)
  return Type::reference(file.read_strong_type<Table_Id>());
 else
  return Type(type_id);
}

/////////////////////////////////////////////////////////////////////////////
std::string joedb::Readonly_Journal::safe_read_string()
/////////////////////////////////////////////////////////////////////////////
{
 return file.safe_read_string(checkpoint_position);
}

#define TYPE_MACRO(cpp_type, return_type, type_id, read_method, W)\
void joedb::Readonly_Journal::read_vector_of_##type_id(cpp_type *data, size_t size)\
{\
 for (size_t i = 0; i < size; i++)\
  data[i] = read_method();\
}
#define TYPE_MACRO_NO_INT
#define TYPE_MACRO_NO_FLOAT
#include "joedb/TYPE_MACRO.h"

#define TYPE_MACRO(cpp_type, return_type, type_id, read_method, W)\
void joedb::Readonly_Journal::read_vector_of_##type_id(cpp_type *data, size_t size)\
{\
 file.read_data((char *)data, size * sizeof(cpp_type));\
}
#define TYPE_MACRO_NO_STRING
#define TYPE_MACRO_NO_REFERENCE
#define TYPE_MACRO_NO_BLOB
#include "joedb/TYPE_MACRO.h"
