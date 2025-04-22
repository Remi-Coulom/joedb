#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/Buffered_File.h"
#include "joedb/journal/Header.h"
#include "joedb/error/Exception.h"
#include "joedb/error/Destructor_Logger.h"

/////////////////////////////////////////////////////////////////////////////
joedb::Writable_Journal::Writable_Journal
/////////////////////////////////////////////////////////////////////////////
(
 Journal_Construction_Lock &lock,
 Check check,
 Commit_Level commit_level
):
 Readonly_Journal(lock, check),
 Writable(commit_level),
 current_commit_level(Commit_Level::no_commit)
{
 if (file.is_readonly())
 {
  throw Exception("Cannot create Writable_Journal with read-only file");
 }
 else if (lock.is_creating_new())
 {
  Header header;
  header.checkpoint.fill(header_size);
  header.version = Readonly_Journal::version_number;
  header.signature = Header::joedb;
  file.pwrite((const char *)&header, Header::size, 0);
  file.set_position(Header::size);
 }
 else
 {
  throw Exception("this conversion version can create new only");
 }
}

/////////////////////////////////////////////////////////////////////////////
joedb::Writable_Journal::Writable_Journal
/////////////////////////////////////////////////////////////////////////////
(
 Journal_Construction_Lock &&lock,
 Check check,
 Commit_Level commit_level
):
 Writable_Journal(lock, check, commit_level)
{
}

/////////////////////////////////////////////////////////////////////////////
joedb::Writable_Journal::Writable_Journal
/////////////////////////////////////////////////////////////////////////////
(
 Buffered_File &file,
 Check check,
 Commit_Level commit_level
):
 Writable_Journal(Journal_Construction_Lock(file), check, commit_level)
{
}

/////////////////////////////////////////////////////////////////////////////
int64_t joedb::Writable_Journal::pull_from
/////////////////////////////////////////////////////////////////////////////
(
 const Readonly_Journal &journal,
 const int64_t until_checkpoint
)
{
 const int64_t source_checkpoint = std::min
 (
  journal.get_checkpoint_position(),
  until_checkpoint
 );

 if (checkpoint_position < source_checkpoint)
 {
  const int64_t initial_position = get_position();

  journal.get_file().copy_to
  (
   file,
   checkpoint_position,
   source_checkpoint - checkpoint_position
  );

  default_checkpoint();

  set_position(initial_position);
 }

 return checkpoint_position;
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
  file.flush();

  checkpoint_index ^= 1;
  checkpoint_position = file.get_position();
  current_commit_level = commit_level;

  {
   file.exclusive_lock_head();

   file.pwrite
   (
    reinterpret_cast<const char *>(&checkpoint_position),
    sizeof(checkpoint_position),
    int64_t(sizeof(checkpoint_position)) * (2 * checkpoint_index)
   );

   if (commit_level > Commit_Level::no_commit)
    file.flush_and_sync();

   file.pwrite
   (
    reinterpret_cast<const char *>(&checkpoint_position),
    sizeof(checkpoint_position),
    int64_t(sizeof(checkpoint_position)) * (2 * checkpoint_index + 1)
   );

   if (commit_level > Commit_Level::half_commit)
    file.flush_and_sync();

   file.unlock_head();
  }

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
 file.compact_write<>(to_underlying(table_id));
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
 file.compact_write<>(to_underlying(table_id));
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
 file.compact_write<>(to_underlying(table_id));
 file.write_string(name);
 file.write<Type_Id_Storage>(Type_Id_Storage(type.get_type_id()));
 if (type.get_type_id() == Type::Type_Id::reference)
  file.compact_write<>(to_underlying(type.get_table_id()));
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
 file.compact_write<>(to_underlying(table_id));
 file.compact_write<>(to_underlying(field_id));
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
 file.compact_write<>(to_underlying(table_id));
 file.compact_write<>(to_underlying(field_id));
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
  file.compact_write<>(to_underlying(table_id));
  file.compact_write<>(to_underlying(record_id));
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
 size_t size
)
{
 file.write<operation_t>(operation_t::insert_vector);
 file.compact_write<>(to_underlying(table_id));
 file.compact_write<>(to_underlying(record_id));
 file.compact_write<>(size);

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
 file.compact_write<>(to_underlying(table_id));
 file.compact_write<>(to_underlying(record_id));
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
  file.compact_write<>(to_underlying(field_id));
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
  ++record_of_last_operation;
 }
 else
 {
  file.write<operation_t>(operation);
  file.compact_write<>(to_underlying(table_id));
  file.compact_write<>(to_underlying(record_id));
  file.compact_write<>(to_underlying(field_id));
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
 size_t size,\
 const type *value\
)\
{\
 file.write<operation_t>(operation_t::update_vector_##type_id);\
 file.compact_write<>(to_underlying(table_id));\
 file.compact_write<>(to_underlying(record_id));\
 file.compact_write<>(to_underlying(field_id));\
 file.compact_write<>(size);\
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
 file.compact_write<size_t>(data.size());
 const int64_t blob_position = get_position();
 file.flush();
 file.sequential_write(data.data(), data.size());
 return Blob(blob_position, data.size());
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Writable_Journal::lock_pull()
/////////////////////////////////////////////////////////////////////////////
{
 if (file.is_shared())
 {
  file.exclusive_lock_tail();
  pull_without_locking();
 }
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Writable_Journal::unlock() noexcept
/////////////////////////////////////////////////////////////////////////////
{
 if (file.is_shared())
  file.unlock_tail();
}

/////////////////////////////////////////////////////////////////////////////
joedb::Writable_Journal::~Writable_Journal()
/////////////////////////////////////////////////////////////////////////////
{
 if (ahead_of_checkpoint() > 0)
  Destructor_Logger::write("Ahead_of_checkpoint in Writable_Journal destructor");
}
