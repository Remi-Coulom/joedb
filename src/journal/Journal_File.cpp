#include "joedb/Journal_File.h"
#include "joedb/Generic_File.h"
#include "Exception.h"

#include <vector>
#include <stdexcept>

const uint32_t joedb::Journal_File::version_number = 0x00000004;
const uint32_t joedb::Journal_File::compatible_version = 0x00000004;
const uint64_t joedb::Journal_File::header_size = 41;

#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
#define SAFE_MAX_SIZE 1000000
#else
#define SAFE_MAX_SIZE checkpoint_position
#endif

/////////////////////////////////////////////////////////////////////////////
joedb::Journal_File::Journal_File(Generic_File &file):
/////////////////////////////////////////////////////////////////////////////
 file(file),
 checkpoint_index(0),
 checkpoint_position(0),
 current_commit_level(0),
 table_of_last_operation(0),
 record_of_last_operation(0),
 field_of_last_update(0)
{
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
   throw Exception("File does not start by 'joedb'");
  }
  else
  {
   //
   // Check version number
   //
   const uint32_t version = file.read<uint32_t>();
   if (version < compatible_version || version > version_number)
    throw Exception("Unsupported format version");

   //
   // Find the most recent checkpoint
   //
   uint64_t pos[4];
   for (int i = 0; i < 4; i++)
    pos[i] = file.read<uint64_t>();

   if (pos[0] != pos[1] || pos[2] != pos[3])
    throw Exception("Checkpoint mismatch");

   checkpoint_position = 0;

   for (unsigned i = 0; i < 2; i++)
    if (pos[2 * i] == pos[2 * i + 1] && pos[2 * i] > checkpoint_position)
    {
     checkpoint_position = pos[2 * i];
     checkpoint_index = i;
    }

   if (checkpoint_position < header_size)
    throw Exception("Checkpoint too small");

   //
   // Compare to file size (if available)
   //
   int64_t file_size = file.get_size();
   if (file_size > 0 && uint64_t(file_size) != checkpoint_position)
    throw Exception("Checkpoint different from file size");
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
 if (ahead_of_checkpoint() || commit_level > current_commit_level)
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
void joedb::Journal_File::replay_log(Writeable &writeable)
/////////////////////////////////////////////////////////////////////////////
{
 rewind();
 play_until_checkpoint(writeable);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::rewind()
/////////////////////////////////////////////////////////////////////////////
{
 file.set_position(header_size);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::play_until(Writeable &writeable, uint64_t end)
/////////////////////////////////////////////////////////////////////////////
{
 while(file.get_position() < end)
 {
  switch(file.read<operation_t>())
  {
   case operation_t::end_of_file:
    if (file.get_position() != checkpoint_position)
     throw Exception("Unexpected end of file");

   case operation_t::create_table:
   {
    std::string name = safe_read_string();
    writeable.create_table(name);
   }
   break;

   case operation_t::drop_table:
   {
    Table_Id table_id = file.compact_read<Table_Id>();
    writeable.drop_table(table_id);
   }
   break;

   case operation_t::rename_table:
   {
    Table_Id table_id = file.compact_read<Table_Id>();
    std::string name = safe_read_string();
    writeable.rename_table(table_id, name);
   }
   break;

   case operation_t::add_field:
   {
    Table_Id table_id = file.compact_read<Table_Id>();
    std::string name = safe_read_string();
    Type type = read_type();
    writeable.add_field(table_id, name, type);
   }
   break;

   case operation_t::drop_field:
   {
    Table_Id table_id = file.compact_read<Table_Id>();
    Field_Id field_id = file.compact_read<Field_Id>();
    writeable.drop_field(table_id, field_id);
   }
   break;

   case operation_t::rename_field:
   {
    Table_Id table_id = file.compact_read<Table_Id>();
    Field_Id field_id = file.compact_read<Field_Id>();
    std::string name = safe_read_string();
    writeable.rename_field(table_id, field_id, name);
   }
   break;

   case operation_t::insert_into:
   {
    Table_Id table_id = file.compact_read<Table_Id>();
    Record_Id record_id = file.compact_read<Record_Id>();
    writeable.insert_into(table_id, record_id);
    table_of_last_operation = table_id;
    record_of_last_operation = record_id;
   }
   break;

   case operation_t::insert_vector:
   {
    Table_Id table_id = file.compact_read<Table_Id>();
    Record_Id record_id = file.compact_read<Record_Id>();
    Record_Id size = file.compact_read<Record_Id>();
    writeable.insert_vector(table_id, record_id, size);
    table_of_last_operation = table_id;
    record_of_last_operation = record_id;
   }
   break;

   case operation_t::append:
    writeable.insert_into(table_of_last_operation,
                          ++record_of_last_operation);
   break;

   case operation_t::delete_from:
   {
    Table_Id table_id = file.compact_read<Table_Id>();
    Record_Id record_id = file.compact_read<Record_Id>();
    writeable.delete_from(table_id, record_id);
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
    writeable.update_##type_id(table_of_last_operation,\
                                    record_of_last_operation,\
                                    field_of_last_update,\
                                    value);\
   }\
   break;\
\
   case operation_t::update_vector_##type_id:\
   {\
    table_of_last_operation = file.compact_read<Table_Id>();\
    record_of_last_operation = file.compact_read<Record_Id>();\
    field_of_last_update = file.compact_read<Field_Id>();\
    Record_Id size = file.compact_read<Record_Id>();\
    if (size > SAFE_MAX_SIZE || size < 0)\
     throw Exception("update_vector too big");\
    std::vector<cpp_type> buffer(size);\
    for (size_t i = 0; i < size; i++)\
     buffer[i] = read_method();\
    writeable.update_vector_##type_id(table_of_last_operation,\
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
    writeable.custom(name);
   }
   break;

   case operation_t::comment:
   {
    std::string comment = safe_read_string();
    writeable.comment(comment);
   }
   break;

   case operation_t::timestamp:
   {
    int64_t timestamp = file.read<int64_t>();
    writeable.timestamp(timestamp);
   }
   break;

   case operation_t::valid_data:
    writeable.valid_data();
   break;

   default:
    throw Exception("Unexpected operation");
  }
 }

 file.set_position(end); // get ready for writing
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::create_table(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::create_table);
 file.write_string(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::drop_table(Table_Id table_id)
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::drop_table);
 file.compact_write<Table_Id>(table_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::rename_table
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 const std::string &name
)
{
 file.write<operation_t>(operation_t::rename_table);
 file.compact_write<Table_Id>(table_id);
 file.write_string(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::add_field
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 const std::string &name,
 Type type
)
{
 file.write<operation_t>(operation_t::add_field);
 file.compact_write<Table_Id>(table_id);
 file.write_string(name);
 file.write<Type::Type_Id>(type.get_type_id());
 if (type.get_type_id() == Type::Type_Id::reference)
  file.compact_write<Table_Id>(type.get_table_id());
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::drop_field
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 Field_Id field_id
)
{
 file.write<operation_t>(operation_t::drop_field);
 file.compact_write<Table_Id>(table_id);
 file.compact_write<Field_Id>(field_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::rename_field
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 Field_Id field_id,
 const std::string &name
)
{
 file.write<operation_t>(operation_t::rename_field);
 file.compact_write<Table_Id>(table_id);
 file.compact_write<Field_Id>(field_id);
 file.write_string(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::custom(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::custom);
 file.write_string(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::comment(const std::string &comment)
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::comment);
 file.write_string(comment);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::timestamp(int64_t timestamp)
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::timestamp);
 file.write<int64_t>(timestamp);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::valid_data()
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::valid_data);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::insert_into
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 Record_Id record_id
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
  file.compact_write<Table_Id>(table_id);
  file.compact_write<Record_Id>(record_id);
 }

 table_of_last_operation = table_id;
 record_of_last_operation = record_id;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::insert_vector
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 Record_Id record_id,
 Record_Id size
)
{
 file.write<operation_t>(operation_t::insert_vector);
 file.compact_write<Table_Id>(table_id);
 file.compact_write<Record_Id>(record_id);
 file.compact_write<Record_Id>(size);

 table_of_last_operation = table_id;
 record_of_last_operation = record_id;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Journal_File::delete_from
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 Record_Id record_id
)
{
 file.write<operation_t>(operation_t::delete_from);
 file.compact_write<Table_Id>(table_id);
 file.compact_write<Record_Id>(record_id);
}

/////////////////////////////////////////////////////////////////////////////
#define TYPE_MACRO(type, return_type, type_id, R, write_method)\
void joedb::Journal_File::update_##type_id\
(\
 Table_Id table_id,\
 Record_Id record_id,\
 Field_Id field_id,\
 return_type value\
)\
{\
 if (table_id == table_of_last_operation &&\
     record_id == record_of_last_operation)\
 {\
  file.write<operation_t>(operation_t::update_last_##type_id);\
  file.compact_write<Field_Id>(field_id);\
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
  file.compact_write<Table_Id>(table_id);\
  file.compact_write<Record_Id>(record_id);\
  file.compact_write<Field_Id>(field_id);\
  table_of_last_operation = table_id;\
  record_of_last_operation = record_id;\
  field_of_last_update = field_id;\
 }\
 file.write_method(value);\
}\
void joedb::Journal_File::update_vector_##type_id\
(\
 Table_Id table_id,\
 Record_Id record_id,\
 Field_Id field_id,\
 Record_Id size,\
 const type *value\
)\
{\
 file.write<operation_t>(operation_t::update_vector_##type_id);\
 file.compact_write<Table_Id>(table_id);\
 file.compact_write<Record_Id>(record_id);\
 file.compact_write<Field_Id>(field_id);\
 file.compact_write<Record_Id>(size);\
 table_of_last_operation = table_id;\
 record_of_last_operation = record_id;\
 field_of_last_update = field_id;\
 for (Record_Id i = 0; i < size; i++)\
  file.write_method(value[i]);\
}
#include "joedb/TYPE_MACRO.h"
#undef TYPE_MACRO

/////////////////////////////////////////////////////////////////////////////
joedb::Type joedb::Journal_File::read_type()
/////////////////////////////////////////////////////////////////////////////
{
 Type::Type_Id type_id = file.read<Type::Type_Id>();
 if (type_id == Type::Type_Id::reference)
  return Type::reference(file.compact_read<Table_Id>());
 else
  return Type(type_id);
}

/////////////////////////////////////////////////////////////////////////////
std::string joedb::Journal_File::safe_read_string()
/////////////////////////////////////////////////////////////////////////////
{
 return file.safe_read_string(SAFE_MAX_SIZE);
}

/////////////////////////////////////////////////////////////////////////////
joedb::Journal_File::~Journal_File()
/////////////////////////////////////////////////////////////////////////////
{
 checkpoint(0);
}
