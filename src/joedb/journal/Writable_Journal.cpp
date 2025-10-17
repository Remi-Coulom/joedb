#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/File_Buffer.h"
#include "joedb/error/Exception.h"
#include "joedb/error/Destructor_Logger.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 Writable_Journal::Writable_Journal(Journal_Construction_Lock &lock):
 /////////////////////////////////////////////////////////////////////////////
  Readonly_Journal(lock.set_for_writable_journal())
 {
  if (file.is_readonly())
   throw Exception("Cannot create Writable_Journal with read-only file");
  else if (lock.size == 0)
  {
   Header header;
   header.checkpoint.fill(Header::size);
   header.version = format_version;
   header.signature = Header::joedb;
   file_buffer.File_Iterator::write((const char *)(&header), Header::size);
  }
  else if
  (
   lock.size > 0 &&
   lock.size > checkpoint_position &&
   lock.recovery != Recovery::overwrite
  )
  {
   throw Exception
   (
    "Checkpoint (" + std::to_string(checkpoint_position) +
    ") is smaller than file size (" + std::to_string(lock.size) +
    "). This file may contain an aborted transaction. "
    "'joedb_push file.joedb file fixed.joedb' can be used to truncate it."
   );
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 int64_t Writable_Journal::pull_from
 /////////////////////////////////////////////////////////////////////////////
 (
  const Readonly_Journal &journal,
  const int64_t until
 )
 {
  if (checkpoint_position < until)
  {
   const int64_t size = until - checkpoint_position;
   journal.get_file().copy_to(file, checkpoint_position, size);
   soft_checkpoint_at(until);
  }

  return checkpoint_position;
 }

 /////////////////////////////////////////////////////////////////////////////
 int64_t Writable_Journal::ahead_of_checkpoint() const noexcept
 /////////////////////////////////////////////////////////////////////////////
 {
  return file_buffer.get_position() - checkpoint_position;
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::start_writing(int64_t position)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (position != checkpoint_position)
   throw Exception("writing must start at checkpoint position");
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::end_writing(int64_t position)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (position != file_buffer.get_position())
   throw Exception("end_writing position is not matching file position");
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::soft_checkpoint_at(int64_t position)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (checkpoint_position >= position)
   return;

  file_buffer.flush();

  JOEDB_DEBUG_ASSERT(file.get_size() < 0 || position <= file.get_size());

  soft_index ^= 1;
  checkpoint_position = position;

  Abstract_File::Head_Exclusive_Lock lock(file);

  const int64_t neg = -checkpoint_position;

  file.pwrite
  (
   reinterpret_cast<const char *>(&neg),
   sizeof(neg),
   int64_t(sizeof(neg)) * (2 * (hard_index ^ 1) + soft_index)
  );
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::hard_checkpoint_at(int64_t position)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (hard_checkpoint_position >= position)
   return;

  file_buffer.flush();

  hard_index ^= 1;
  hard_checkpoint_position = checkpoint_position = position;

  Abstract_File::Head_Exclusive_Lock lock(file);

  file.pwrite
  (
   reinterpret_cast<const char *>(&checkpoint_position),
   sizeof(checkpoint_position),
   int64_t(sizeof(checkpoint_position)) * (2 * hard_index)
  );

  file.sync();

  file.pwrite
  (
   reinterpret_cast<const char *>(&checkpoint_position),
   sizeof(checkpoint_position),
   int64_t(sizeof(checkpoint_position)) * (2 * hard_index + 1)
  );

  file.datasync();
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::soft_checkpoint()
 /////////////////////////////////////////////////////////////////////////////
 {
  soft_checkpoint_at(file_buffer.get_position());
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::hard_checkpoint()
 /////////////////////////////////////////////////////////////////////////////
 {
  hard_checkpoint_at(file_buffer.get_position());
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::create_table(const std::string &name)
 /////////////////////////////////////////////////////////////////////////////
 {
  file_buffer.write<operation_t>(operation_t::create_table);
  file_buffer.write_string(name);
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::drop_table(Table_Id table_id)
 /////////////////////////////////////////////////////////////////////////////
 {
  file_buffer.write<operation_t>(operation_t::drop_table);
  file_buffer.compact_write<>(to_underlying(table_id));
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::rename_table
 /////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  const std::string &name
 )
 {
  file_buffer.write<operation_t>(operation_t::rename_table);
  file_buffer.compact_write<>(to_underlying(table_id));
  file_buffer.write_string(name);
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::add_field
 /////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  const std::string &name,
  Type type
 )
 {
  file_buffer.write<operation_t>(operation_t::add_field);
  file_buffer.compact_write<>(to_underlying(table_id));
  file_buffer.write_string(name);
  file_buffer.write<Type_Id_Storage>(Type_Id_Storage(type.get_type_id()));
  if (type.get_type_id() == Type::Type_Id::reference)
   file_buffer.compact_write<>(to_underlying(type.get_table_id()));
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::drop_field
 /////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Field_Id field_id
 )
 {
  file_buffer.write<operation_t>(operation_t::drop_field);
  file_buffer.compact_write<>(to_underlying(table_id));
  file_buffer.compact_write<>(to_underlying(field_id));
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::rename_field
 /////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Field_Id field_id,
  const std::string &name
 )
 {
  file_buffer.write<operation_t>(operation_t::rename_field);
  file_buffer.compact_write<>(to_underlying(table_id));
  file_buffer.compact_write<>(to_underlying(field_id));
  file_buffer.write_string(name);
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::custom(const std::string &name)
 /////////////////////////////////////////////////////////////////////////////
 {
  file_buffer.write<operation_t>(operation_t::custom);
  file_buffer.write_string(name);
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::comment(const std::string &comment)
 /////////////////////////////////////////////////////////////////////////////
 {
  file_buffer.write<operation_t>(operation_t::comment);
  file_buffer.write_string(comment);
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::timestamp(int64_t timestamp)
 /////////////////////////////////////////////////////////////////////////////
 {
  file_buffer.write<operation_t>(operation_t::timestamp);
  file_buffer.write<int64_t>(timestamp);
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::valid_data()
 /////////////////////////////////////////////////////////////////////////////
 {
  file_buffer.write<operation_t>(operation_t::valid_data);
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::insert_into
 /////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id
 )
 {
  if (table_id == table_of_last_operation &&
      record_id == record_of_last_operation + 1)
  {
   file_buffer.write<operation_t>(operation_t::append);
  }
  else
  {
   file_buffer.write<operation_t>(operation_t::insert_into);
   file_buffer.compact_write<>(to_underlying(table_id));
   file_buffer.write_reference(record_id);
  }

  table_of_last_operation = table_id;
  record_of_last_operation = record_id;
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::delete_from
 /////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id
 )
 {
  file_buffer.write<operation_t>(operation_t::delete_from);
  file_buffer.compact_write<>(to_underlying(table_id));
  file_buffer.write_reference(record_id);
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::insert_vector
 /////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id,
  size_t size
 )
 {
  file_buffer.write<operation_t>(operation_t::insert_vector);
  file_buffer.compact_write<>(to_underlying(table_id));
  file_buffer.write_reference(record_id);
  file_buffer.compact_write<>(size);

  table_of_last_operation = table_id;
  record_of_last_operation = record_id;
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::delete_vector
 /////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id,
  size_t size
 )
 {
  file_buffer.write<operation_t>(operation_t::delete_vector);
  file_buffer.compact_write<>(to_underlying(table_id));
  file_buffer.write_reference(record_id);
  file_buffer.compact_write<>(size);
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::generic_update
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

   file_buffer.write<operation_t>(operation_t(int(operation) + last));
   file_buffer.compact_write<>(to_underlying(field_id));
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
   file_buffer.write<operation_t>(operation_t(int(operation) + next));
   ++record_of_last_operation;
  }
  else
  {
   file_buffer.write<operation_t>(operation);
   file_buffer.compact_write<>(to_underlying(table_id));
   file_buffer.write_reference(record_id);
   file_buffer.compact_write<>(to_underlying(field_id));
   table_of_last_operation = table_id;
   record_of_last_operation = record_id;
   field_of_last_update = field_id;
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 #define TYPE_MACRO(type, return_type, type_id, R, write_method)\
 void Writable_Journal::update_##type_id\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id,\
  return_type value\
 )\
 {\
  generic_update(table_id, record_id, field_id, operation_t::update_##type_id);\
  file_buffer.write_method(value);\
 }\
 void Writable_Journal::update_vector_##type_id\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id,\
  size_t size,\
  const type *value\
 )\
 {\
  file_buffer.write<operation_t>(operation_t::update_vector_##type_id);\
  file_buffer.compact_write<>(to_underlying(table_id));\
  file_buffer.write_reference(record_id);\
  file_buffer.compact_write<>(to_underlying(field_id));\
  file_buffer.compact_write<>(size);\
  table_of_last_operation = table_id;\
  record_of_last_operation = record_id;\
  field_of_last_update = field_id;\
 \
  if constexpr\
  (\
   Type::Type_Id::type_id == Type::Type_Id::blob ||\
   Type::Type_Id::type_id == Type::Type_Id::string ||\
   Type::Type_Id::type_id == Type::Type_Id::reference\
  )\
  {\
   for (size_t i = 0; i < size; i++)\
    file_buffer.write_method(value[i]);\
  }\
  else\
   file_buffer.write_data((const char *)value, size * sizeof(type));\
 }
 #include "joedb/TYPE_MACRO.h"

 /////////////////////////////////////////////////////////////////////////////
 Blob Writable_Journal::write_blob
 /////////////////////////////////////////////////////////////////////////////
 (
  const std::string &data
 )
 {
  file_buffer.write<operation_t>(operation_t::blob);
  file_buffer.compact_write<size_t>(data.size());
  const int64_t blob_position = get_position();
  file_buffer.flush();
  file_buffer.File_Iterator::write(data.data(), data.size());
  return Blob(blob_position, int64_t(data.size()));
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::lock_pull()
 /////////////////////////////////////////////////////////////////////////////
 {
  if (file.is_shared())
  {
   file.exclusive_lock_tail();
   pull_without_locking();
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::unlock() noexcept
 /////////////////////////////////////////////////////////////////////////////
 {
  if (file.is_shared())
   file.unlock_tail();
 }

 /////////////////////////////////////////////////////////////////////////////
 void Writable_Journal::touch()
 /////////////////////////////////////////////////////////////////////////////
 {
  file.pwrite
  (
   Header::joedb.data(),
   Header::joedb.size(),
   offsetof(Header, signature)
  );
 }

 /////////////////////////////////////////////////////////////////////////////
 Writable_Journal::~Writable_Journal()
 /////////////////////////////////////////////////////////////////////////////
 {
  if (ahead_of_checkpoint() > 0)
   Destructor_Logger::warning("Writable_Journal: ahead of checkpoint");
 }

 /////////////////////////////////////////////////////////////////////////////
 Tail_Exclusive_Lock::Tail_Exclusive_Lock(Writable_Journal &journal):
 /////////////////////////////////////////////////////////////////////////////
  journal(journal)
 {
  if (journal.get_position() > journal.get_checkpoint())
   throw Exception("locking journal with uncheckpointed data (try rollback?)");
  journal.lock_pull();
 }

 /////////////////////////////////////////////////////////////////////////////
 Tail_Exclusive_Lock::~Tail_Exclusive_Lock()
 /////////////////////////////////////////////////////////////////////////////
 {
  journal.unlock();
 }
}
