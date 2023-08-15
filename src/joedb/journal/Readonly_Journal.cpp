#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/Generic_File.h"
#include "joedb/Exception.h"

#include <sstream>

constexpr uint32_t joedb::Readonly_Journal::version_number;
constexpr uint32_t joedb::Readonly_Journal::compatible_version;
constexpr int64_t joedb::Readonly_Journal::header_size;

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
void joedb::Readonly_Journal::construct(Check check)
/////////////////////////////////////////////////////////////////////////////
{
 file.set_position(0);

 //
 // Check the format of an existing joedb file
 //
 if (file.get_mode() != Open_Mode::create_new)
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
   read_checkpoint(check_flag(check, Check::checkpoint_mismatch));

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
     if (check_flag(check, Check::big_size) && file_size > checkpoint_position)
      throw Exception
      (
       "Checkpoint is smaller than file size. "
       "This file may contain an aborted transaction. "
       "'joedb_push file.joedb file fixed.joedb' can be used to truncate it."
      );

     if (check_flag(check, Check::small_size) && file_size < checkpoint_position)
      throw Exception("Checkpoint is bigger than file size");
    }
   }
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
joedb::Readonly_Journal::Readonly_Journal
/////////////////////////////////////////////////////////////////////////////
(
 Generic_File &file,
 Check check
):
 file(file),
 file_version(0),
 checkpoint_index(0),
 checkpoint_position(0),
 table_of_last_operation(0),
 record_of_last_operation(0),
 field_of_last_update(0)
{
 if (file.is_shared())
 {
  file.shared_transaction([this, check]()
  {
   construct(check);
  });
 }
 else
  construct(check);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::read_checkpoint(bool strict)
/////////////////////////////////////////////////////////////////////////////
{
 int64_t pos[4];
 for (int i = 0; i < 4; i++)
  pos[i] = file.read<int64_t>();

 if (strict)
 {
  if (pos[0] != pos[1] || pos[2] != pos[3])
   throw Exception("Checkpoint mismatch");
 }

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
void joedb::Readonly_Journal::refresh_checkpoint()
/////////////////////////////////////////////////////////////////////////////
{
 const int64_t old_position = file.get_position();
 constexpr int64_t checkpoint_offset = 5 + 4;
 file.set_position(checkpoint_offset);
 read_checkpoint(false);
 file.set_position(old_position);
}

/////////////////////////////////////////////////////////////////////////////
std::vector<char> joedb::Readonly_Journal::get_raw_tail
/////////////////////////////////////////////////////////////////////////////
(
 int64_t starting_position
) const
{
 Async_Reader reader = get_tail_reader(starting_position);
 std::vector<char> result(reader.get_remaining());
 reader.read(result.data(), result.size());

 // TODO:
 // This function must not exist: step-by-step copy instead in File Connections

 return result;
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
 writable.checkpoint(Commit_Level::full_commit);
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
  case operation_t::end_of_file:
   if (file.get_position() != checkpoint_position)
    throw Exception("Unexpected end of file");
  break;

  case operation_t::create_table:
  {
   std::string name = safe_read_string();
   writable.create_table(name);
  }
  break;

  case operation_t::drop_table:
  {
   const Table_Id table_id = file.compact_read<Table_Id>();
   writable.drop_table(table_id);
  }
  break;

  case operation_t::rename_table:
  {
   const Table_Id table_id = file.compact_read<Table_Id>();
   std::string name = safe_read_string();
   writable.rename_table(table_id, name);
  }
  break;

  case operation_t::add_field:
  {
   const Table_Id table_id = file.compact_read<Table_Id>();
   std::string name = safe_read_string();
   const Type type = read_type();
   writable.add_field(table_id, name, type);
  }
  break;

  case operation_t::drop_field:
  {
   const Table_Id table_id = file.compact_read<Table_Id>();
   const Field_Id field_id = file.compact_read<Field_Id>();
   writable.drop_field(table_id, field_id);
  }
  break;

  case operation_t::rename_field:
  {
   const Table_Id table_id = file.compact_read<Table_Id>();
   const Field_Id field_id = file.compact_read<Field_Id>();
   std::string name = safe_read_string();
   writable.rename_field(table_id, field_id, name);
  }
  break;

  case operation_t::insert_into:
  {
   const Table_Id table_id = file.compact_read<Table_Id>();
   const Record_Id record_id = file.compact_read<Record_Id>();
   writable.insert_into(table_id, record_id);
   table_of_last_operation = table_id;
   record_of_last_operation = record_id;
  }
  break;

  case operation_t::insert_vector:
  {
   const Table_Id table_id = file.compact_read<Table_Id>();
   const Record_Id record_id = file.compact_read<Record_Id>();
   const Record_Id size = file.compact_read<Record_Id>();
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
   const Table_Id table_id = file.compact_read<Table_Id>();
   const Record_Id record_id = file.compact_read<Record_Id>();
   writable.delete_from(table_id, record_id);
  }
  break;

  #define TYPE_MACRO(cpp_type, return_type, type_id, read_method, W)\
  case operation_t::update_##type_id:\
   table_of_last_operation = file.compact_read<Table_Id>();\
   record_of_last_operation = file.compact_read<Record_Id>();\
   field_of_last_update = file.compact_read<Field_Id>();\
   perform_update_##type_id(writable);\
  break;\
\
  case operation_t::update_last_##type_id:\
   field_of_last_update = file.compact_read<Field_Id>();\
   perform_update_##type_id(writable);\
  break;\
\
  case operation_t::update_next_##type_id:\
   record_of_last_operation++;\
   perform_update_##type_id(writable);\
  break;
  #include "joedb/TYPE_MACRO.h"

  #define TYPE_MACRO(cpp_type, return_type, type_id, read_method, W)\
  case operation_t::update_vector_##type_id:\
  {\
   table_of_last_operation = file.compact_read<Table_Id>();\
   record_of_last_operation = file.compact_read<Record_Id>();\
   field_of_last_update = file.compact_read<Field_Id>();\
   const Record_Id size = file.compact_read<Record_Id>();\
   if (int64_t(size) > checkpoint_position)\
    throw Exception("update_vector too big");\
   Record_Id capacity;\
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
   else if (record_of_last_operation <= 0 || record_of_last_operation + size - 1 > capacity)\
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
   const int64_t blob_position = get_position();
   writable.on_blob(Blob(blob_position), file);

   if (writable.wants_blobs())
   {
    writable.write_blob_data(safe_read_string());
   }
   else
   {
    const size_t size = file.compact_read<size_t>();
    file.set_position(file.get_position() + int64_t(size));
   }
  }
  break;

  default:
  {
   std::ostringstream error;
   error << "Unexpected operation: file.get_position() = ";
   error << file.get_position();
   throw Exception(error.str());
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
joedb::Type joedb::Readonly_Journal::read_type()
/////////////////////////////////////////////////////////////////////////////
{
 const Type::Type_Id type_id = Type::Type_Id(file.read<Type_Id_Storage>());
 if (type_id == Type::Type_Id::reference)
  return Type::reference(file.compact_read<Table_Id>());
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
