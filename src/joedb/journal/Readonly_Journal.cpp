#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/Buffered_File.h"
#include "joedb/error/Exception.h"

#include <vector>

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
void joedb::Readonly_Journal::reset_context()
/////////////////////////////////////////////////////////////////////////////
{
 table_of_last_operation = Table_Id(0);
 record_of_last_operation = Record_Id(0);
 field_of_last_update = Field_Id(0);
}

/////////////////////////////////////////////////////////////////////////////
joedb::Readonly_Journal::Readonly_Journal(Journal_Construction_Lock &lock):
/////////////////////////////////////////////////////////////////////////////
 file(lock.file),
 hard_index(0),
 soft_index(0),
 checkpoint_position(Header::size)
{
 if (lock.size != 0)
 {
  Header header;
  if (file.pread((char *)(&header), Header::size, 0) < Header::size)
   file.reading_past_end_of_file();

  file.set_position(Header::size);

  if (header.signature != Header::joedb && !lock.ignore_errors)
   throw Exception("missing joedb signature");

  if (header.version != format_version && !lock.ignore_errors)
   throw Exception("unsupported file format version");

  read_checkpoint(header.checkpoint);

  if (lock.size > Header::ssize && lock.ignore_errors)
   checkpoint_position = lock.size;

  if (lock.size > 0 && lock.size < checkpoint_position)
   throw Exception("Checkpoint is bigger than file size");
 }

 reset_context();
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::read_checkpoint
/////////////////////////////////////////////////////////////////////////////
(
 const std::array<int64_t, 4> &pos
)
{
 int64_t hard_checkpoint = 0;
 int64_t soft_checkpoint = 0;

 for (int i = 0; i < 2; i++)
 {
  if (pos[2 * i] == pos[2 * i + 1] && pos[2 * i] >= hard_checkpoint)
  {
   hard_checkpoint = pos[2 * i];
   hard_index = i;
  }

  for (int j = 0; j < 2; j++)
  {
   if (-pos[2 * i + j] >= soft_checkpoint)
   {
    soft_checkpoint = -pos[2 * i + j];
    hard_index = i ^ 1;
    soft_index = j;
   }
  }
 }

 if (hard_checkpoint > checkpoint_position)
  checkpoint_position = hard_checkpoint;

 if (soft_checkpoint > checkpoint_position)
  checkpoint_position = soft_checkpoint;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::pull_without_locking()
/////////////////////////////////////////////////////////////////////////////
{
 std::array<int64_t, 4> pos;
 file.pread((char *)&pos, sizeof(pos), 0);
 read_checkpoint(pos);
}

/////////////////////////////////////////////////////////////////////////////
int64_t joedb::Readonly_Journal::pull()
/////////////////////////////////////////////////////////////////////////////
{
 const int64_t old_checkpoint = checkpoint_position;

 Buffered_File::Head_Shared_Lock lock(file);
 pull_without_locking();

 return checkpoint_position - old_checkpoint;
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
 writable.start_writing(get_position());
 while(get_position() < checkpoint_position)
 {
  one_step(writable);
  writable.comment(std::to_string(get_position()));
 }
 writable.soft_checkpoint_at(get_position());
 file.flush();
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::rewind()
/////////////////////////////////////////////////////////////////////////////
{
 file.set_position(Header::size);
 reset_context();
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::play_until(Writable &writable, int64_t end)
/////////////////////////////////////////////////////////////////////////////
{
 if (get_position() < end)
 {
  writable.start_writing(get_position());
  while(get_position() < end)
   one_step(writable);
  writable.soft_checkpoint_at(get_position());
 }

 file.flush_for_writing();
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::one_step(Writable &writable)
/////////////////////////////////////////////////////////////////////////////
{
 switch(file.read<operation_t>())
 {
  case operation_t::create_table:
  {
   const std::string name = safe_read_string();
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
   const std::string name = safe_read_string();
   writable.rename_table(table_id, name);
  }
  break;

  case operation_t::add_field:
  {
   const Table_Id table_id = file.read_strong_type<Table_Id>();
   const std::string name = safe_read_string();
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
   const std::string name = safe_read_string();
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

  case operation_t::delete_from:
  {
   const Table_Id table_id = file.read_strong_type<Table_Id>();
   const Record_Id record_id = file.read_strong_type<Record_Id>();
   writable.delete_from(table_id, record_id);
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

  case operation_t::delete_vector:
  {
   const Table_Id table_id = file.read_strong_type<Table_Id>();
   const Record_Id record_id = file.read_strong_type<Record_Id>();
   const size_t size = file.compact_read<size_t>();
   writable.delete_vector(table_id, record_id, size);
  }
  break;

  case operation_t::append:
   writable.insert_into(table_of_last_operation, ++record_of_last_operation);
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
   const int64_t size = file.compact_read<int64_t>();
   writable.on_blob(Blob(get_position(), size));

   if (writable.wants_blob_data() && size < checkpoint_position)
   {
    std::string s(size, 0);
    if (size > 0)
     file.read_data(s.data(), size);
    writable.write_blob(s);
   }
   else
    file.ignore(size);
  }
  break;

  default:
  {
   throw Exception("Unexpected operation: get_position() = " + std::to_string(get_position()));
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
