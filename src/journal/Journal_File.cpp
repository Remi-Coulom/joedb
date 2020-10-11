#include "joedb/Journal_File.h"
#include "joedb/Generic_File.h"
#include "joedb/Exception.h"

#include <vector>

/////////////////////////////////////////////////////////////////////////////
joedb::Journal_File::Journal_File(Generic_File &file):
/////////////////////////////////////////////////////////////////////////////
 Readonly_Journal(file),
 current_commit_level(0)
{
 if (file.get_mode() == Open_Mode::create_new)
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
}

/////////////////////////////////////////////////////////////////////////////
int64_t joedb::Journal_File::ahead_of_checkpoint() const
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
 file.write<Type_Id_Storage>(Type_Id_Storage(type.get_type_id()));
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
joedb::Journal_File::~Journal_File()
/////////////////////////////////////////////////////////////////////////////
{
 checkpoint(0);
}
