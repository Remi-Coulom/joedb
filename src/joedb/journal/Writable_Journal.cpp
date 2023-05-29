#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/Generic_File.h"
#include "joedb/Exception.h"

#include <vector>

/////////////////////////////////////////////////////////////////////////////
joedb::Writable_Journal::Writable_Journal(Generic_File &file):
/////////////////////////////////////////////////////////////////////////////
 Readonly_Journal(file, Check::all),
 current_commit_level(Commit_Level::no_commit)
{
 if (file.get_mode() == Open_Mode::read_existing)
 {
  throw Exception("Cannot create Writable_Journal with read-only file");
 }
 else if (file.get_mode() == Open_Mode::create_new)
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
  checkpoint(Commit_Level::no_commit);
  file.set_mode(Open_Mode::write_existing);
 }
 else if (version_number > file_version)
 {
  file_version = version_number;
  file.set_position(5);
  file.write<uint32_t>(file_version);
  file.set_position(header_size);
 }
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Writable_Journal::append_raw_tail
/////////////////////////////////////////////////////////////////////////////
(
 const char *data,
 size_t size,
 Commit_Level commit_level
)
{
 Tail_Writer tail_writer(*this, commit_level);
 tail_writer.append(data, size);
 tail_writer.finish();
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Writable_Journal::append_raw_tail
/////////////////////////////////////////////////////////////////////////////
(
 const std::vector<char> &data,
 Commit_Level commit_level
)
{
 append_raw_tail(data.data(), data.size(), commit_level);
}

/////////////////////////////////////////////////////////////////////////////
int64_t joedb::Writable_Journal::ahead_of_checkpoint() const noexcept
/////////////////////////////////////////////////////////////////////////////
{
 return file.get_position() - checkpoint_position;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Writable_Journal::checkpoint(joedb::Commit_Level commit_level)
/////////////////////////////////////////////////////////////////////////////
{
 if
 (
  ahead_of_checkpoint() > 0 ||
  (ahead_of_checkpoint() == 0 && commit_level > current_commit_level)
 )
 {
  checkpoint_index ^= 1;
  checkpoint_position = file.get_position();
  current_commit_level = commit_level;

  file.set_position(9 + 16 * checkpoint_index);
  file.write<uint64_t>(uint64_t(checkpoint_position));

  file.flush();
  if (commit_level > Commit_Level::no_commit)
   file.commit();

  file.write<uint64_t>(uint64_t(checkpoint_position));

  file.flush();
  if (commit_level > Commit_Level::half_commit)
   file.commit();

  file.set_position(checkpoint_position);
 }
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Writable_Journal::create_table(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::create_table);
 file.write_string(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Writable_Journal::drop_table(Table_Id table_id)
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::drop_table);
 file.compact_write<Table_Id>(table_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Writable_Journal::rename_table
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
void joedb::Writable_Journal::add_field
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
void joedb::Writable_Journal::drop_field
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
void joedb::Writable_Journal::rename_field
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
void joedb::Writable_Journal::custom(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::custom);
 file.write_string(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Writable_Journal::comment(const std::string &comment)
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::comment);
 file.write_string(comment);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Writable_Journal::timestamp(int64_t timestamp)
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::timestamp);
 file.write<int64_t>(timestamp);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Writable_Journal::valid_data()
/////////////////////////////////////////////////////////////////////////////
{
 file.write<operation_t>(operation_t::valid_data);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Writable_Journal::insert_into
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
void joedb::Writable_Journal::insert_vector
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
void joedb::Writable_Journal::delete_from
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
void joedb::Writable_Journal::generic_update
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 Record_Id record_id,
 Field_Id field_id,
 operation_t operation
)
{
 if
 (
  table_id == table_of_last_operation &&
  record_id == record_of_last_operation
 )
 {
  constexpr int last =
   int(operation_t::update_last_int8) -
   int(operation_t::update_int8);

  file.write<operation_t>(operation_t(int(operation) + last));
  file.compact_write<Field_Id>(field_id);
  field_of_last_update = field_id;
 }
 else if
 (
  table_id == table_of_last_operation &&
  record_id == record_of_last_operation + 1 &&
  field_id == field_of_last_update
 )
 {
  constexpr int next =
   int(operation_t::update_next_int8) -
   int(operation_t::update_int8);
  file.write<operation_t>(operation_t(int(operation) + next));
  record_of_last_operation++;
 }
 else
 {
  file.write<operation_t>(operation);
  file.compact_write<Table_Id>(table_id);
  file.compact_write<Record_Id>(record_id);
  file.compact_write<Field_Id>(field_id);
  table_of_last_operation = table_id;
  record_of_last_operation = record_id;
  field_of_last_update = field_id;
 }
}

/////////////////////////////////////////////////////////////////////////////
#define TYPE_MACRO(type, return_type, type_id, R, write_method)\
void joedb::Writable_Journal::update_##type_id\
(\
 Table_Id table_id,\
 Record_Id record_id,\
 Field_Id field_id,\
 return_type value\
)\
{\
 generic_update(table_id, record_id, field_id, operation_t::update_##type_id);\
 file.write_method(value);\
}\
void joedb::Writable_Journal::update_vector_##type_id\
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
\
 if\
 (\
  Type::Type_Id::type_id == Type::Type_Id::blob ||\
  Type::Type_Id::type_id == Type::Type_Id::string ||\
  Type::Type_Id::type_id == Type::Type_Id::reference\
 )\
 {\
  for (size_t i = 0; i < size; i++)\
   file.write_method(value[i]);\
 }\
 else\
  file.write_data((const char *)value, size * sizeof(type));\
}
#include "joedb/TYPE_MACRO.h"

/////////////////////////////////////////////////////////////////////////////
joedb::Blob joedb::Writable_Journal::write_blob_data
/////////////////////////////////////////////////////////////////////////////
(
 const std::string &data
)
{
 file.write<operation_t>(operation_t::blob);
 return file.write_blob_data(data);
}

/////////////////////////////////////////////////////////////////////////////
joedb::Writable_Journal::~Writable_Journal()
/////////////////////////////////////////////////////////////////////////////
{
 if (ahead_of_checkpoint() > 0)
  postpone_exception("Ahead_of_checkpoint in Writable_Journal destructor");
}
