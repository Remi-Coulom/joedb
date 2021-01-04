#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/Generic_File.h"
#include "joedb/Exception.h"

#include <sstream>

const uint32_t joedb::Readonly_Journal::version_number = 0x00000004;
const uint32_t joedb::Readonly_Journal::compatible_version = 0x00000004;
const int64_t joedb::Readonly_Journal::header_size = 41;

/////////////////////////////////////////////////////////////////////////////
joedb::Readonly_Journal::Readonly_Journal
/////////////////////////////////////////////////////////////////////////////
(
 Generic_File &file,
 bool ignore_errors
):
 file(file),
 checkpoint_index(0),
 checkpoint_position(0),
 table_of_last_operation(0),
 record_of_last_operation(0),
 field_of_last_update(0)
{
 auto format_exception = [ignore_errors](const char *message)
 {
  if (!ignore_errors)
   throw Exception(message);
 };

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
   format_exception("File does not start by 'joedb'");
  }
  else
  {
   //
   // Check version number
   //
   const uint32_t version = file.read<uint32_t>();
   if (version < compatible_version || version > version_number)
    format_exception("Unsupported format version");

   //
   // Find the most recent checkpoint
   //
   int64_t pos[4];
   for (int i = 0; i < 4; i++)
    pos[i] = file.read<int64_t>();

   if (pos[0] != pos[1] || pos[2] != pos[3])
    format_exception("Checkpoint mismatch");

   checkpoint_position = 0;

   for (unsigned i = 0; i < 2; i++)
    if (pos[2 * i] == pos[2 * i + 1] && pos[2 * i] > checkpoint_position)
    {
     if (int64_t(size_t(pos[2 * i])) != pos[2 * i])
      throw Exception("size_t is too small for this file");
     checkpoint_position = pos[2 * i];
     checkpoint_index = i;
    }

   if (checkpoint_position < header_size)
    format_exception("Checkpoint too small");

   //
   // Compare to file size (if available)
   //
   int64_t file_size = file.get_size();

   if (file_size > 0 && file_size != checkpoint_position)
    format_exception("Checkpoint different from file size");

   if (ignore_errors)
    checkpoint_position = file_size;
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
std::vector<char> joedb::Readonly_Journal::get_raw_tail
/////////////////////////////////////////////////////////////////////////////
(
 int64_t starting_position
)
{
 return file.read_tail(starting_position);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::replay_log(Writable &writable)
/////////////////////////////////////////////////////////////////////////////
{
 rewind();
 play_until_checkpoint(writable);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::rewind()
/////////////////////////////////////////////////////////////////////////////
{
 file.set_position(header_size);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Readonly_Journal::seek(int64_t position)
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
   Table_Id table_id = file.compact_read<Table_Id>();
   writable.drop_table(table_id);
  }
  break;

  case operation_t::rename_table:
  {
   Table_Id table_id = file.compact_read<Table_Id>();
   std::string name = safe_read_string();
   writable.rename_table(table_id, name);
  }
  break;

  case operation_t::add_field:
  {
   Table_Id table_id = file.compact_read<Table_Id>();
   std::string name = safe_read_string();
   Type type = read_type();
   writable.add_field(table_id, name, type);
  }
  break;

  case operation_t::drop_field:
  {
   Table_Id table_id = file.compact_read<Table_Id>();
   Field_Id field_id = file.compact_read<Field_Id>();
   writable.drop_field(table_id, field_id);
  }
  break;

  case operation_t::rename_field:
  {
   Table_Id table_id = file.compact_read<Table_Id>();
   Field_Id field_id = file.compact_read<Field_Id>();
   std::string name = safe_read_string();
   writable.rename_field(table_id, field_id, name);
  }
  break;

  case operation_t::insert_into:
  {
   Table_Id table_id = file.compact_read<Table_Id>();
   Record_Id record_id = file.compact_read<Record_Id>();
   writable.insert_into(table_id, record_id);
   table_of_last_operation = table_id;
   record_of_last_operation = record_id;
  }
  break;

  case operation_t::insert_vector:
  {
   Table_Id table_id = file.compact_read<Table_Id>();
   Record_Id record_id = file.compact_read<Record_Id>();
   Record_Id size = file.compact_read<Record_Id>();
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
   Table_Id table_id = file.compact_read<Table_Id>();
   Record_Id record_id = file.compact_read<Record_Id>();
   writable.delete_from(table_id, record_id);
  }
  break;

  #define TYPE_MACRO(cpp_type, return_type, type_id, read_method, W)\
  case operation_t::update_##type_id:\
   table_of_last_operation = file.compact_read<Table_Id>();\
   record_of_last_operation = file.compact_read<Record_Id>();\
   field_of_last_update = file.compact_read<Field_Id>();\
  goto lbl_perform_update_##type_id;\
\
  case operation_t::update_last_##type_id:\
   field_of_last_update = file.compact_read<Field_Id>();\
  goto lbl_perform_update_##type_id;\
\
  case operation_t::update_next_##type_id:\
   record_of_last_operation++;\
  goto lbl_perform_update_##type_id;\
\
  lbl_perform_update_##type_id:\
  {\
   cpp_type value = read_method();\
   writable.update_##type_id\
   (\
    table_of_last_operation,\
    record_of_last_operation,\
    field_of_last_update,\
    value\
   );\
  }\
  break;\
\
  case operation_t::update_vector_##type_id:\
  {\
   table_of_last_operation = file.compact_read<Table_Id>();\
   record_of_last_operation = file.compact_read<Record_Id>();\
   field_of_last_update = file.compact_read<Field_Id>();\
   Record_Id size = file.compact_read<Record_Id>();\
   if (int64_t(size) > checkpoint_position || size < 0)\
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
   std::string name = safe_read_string();
   writable.custom(name);
  }
  break;

  case operation_t::comment:
  {
   std::string comment = safe_read_string();
   writable.comment(comment);
  }
  break;

  case operation_t::timestamp:
  {
   int64_t timestamp = file.read<int64_t>();
   writable.timestamp(timestamp);
  }
  break;

  case operation_t::valid_data:
   writable.valid_data();
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
 Type::Type_Id type_id = Type::Type_Id(file.read<Type_Id_Storage>());
 if (type_id == Type::Type_Id::reference)
  return Type::reference(file.compact_read<Table_Id>());
 else
  return Type(type_id);
}

/////////////////////////////////////////////////////////////////////////////
std::string joedb::Readonly_Journal::safe_read_string()
/////////////////////////////////////////////////////////////////////////////
{
 return file.safe_read_string(size_t(checkpoint_position));
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
 if (Generic_File::is_big_endian() && sizeof(cpp_type) > 1)\
  for (size_t i = 0; i < size; i++)\
   Generic_File::R<cpp_type, sizeof(cpp_type)>::swap(data[i]);\
}
#define TYPE_MACRO_NO_STRING
#define TYPE_MACRO_NO_REFERENCE
#include "joedb/TYPE_MACRO.h"
