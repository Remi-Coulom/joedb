#ifndef joedb_Writable_Journal_declared
#define joedb_Writable_Journal_declared

#include "joedb/Writable.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/Async_Writer.h"
#include <limits>

namespace joedb
{
 /// @ingroup journal
 class Writable_Journal: public Readonly_Journal, public Writable
 {
  private:
   Commit_Level current_commit_level;

   void generic_update
   (
    Table_Id table_id,
    Record_Id record_id,
    Field_Id field_id,
    operation_t operation
   );

   void flush_and_may_sync()
   {
    file.flush();
    if (get_default_commit_level() > Commit_Level::no_commit)
     file.sync();
   }

  public:
   explicit Writable_Journal
   (
    Journal_Construction_Lock &lock,
    Check check,
    Commit_Level commit_level
   );

   explicit Writable_Journal
   (
    Journal_Construction_Lock &&lock,
    Check check,
    Commit_Level commit_level
   );

   explicit Writable_Journal
   (
    Buffered_File &file,
    Check check = Check::all,
    Commit_Level commit_level = Commit_Level::no_commit
   );

   void append()
   {
    file.set_position(checkpoint_position);
   }

   int64_t pull_from
   (
    Readonly_Journal &journal,
    int64_t until_checkpoint = std::numeric_limits<int64_t>::max()
   );

   int64_t ahead_of_checkpoint() const noexcept;

   void flush() final {file.flush();}
   void checkpoint(Commit_Level commit_level) final;

   void create_table(const std::string &name) final;
   void drop_table(Table_Id table_id) final;

   void rename_table
   (
    Table_Id table_id,
    const std::string &name
   ) final;

   void add_field
   (
    Table_Id table_id,
    const std::string &name,
    Type type
   ) final;

   void drop_field
   (
    Table_Id table_id,
    Field_Id field_id
   ) final;

   void rename_field
   (
    Table_Id table_id,
    Field_Id field_id,
    const std::string &name
   ) final;

   void custom(const std::string &name) final;
   void comment(const std::string &comment) final;
   void timestamp(int64_t timestamp) final;
   void valid_data() final;
   void insert_into(Table_Id table_id, Record_Id record_id) final;

   void insert_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    size_t size
   ) final;

   void delete_from(Table_Id table_id, Record_Id record_id) final;

   #define TYPE_MACRO(type, return_type, type_id, read_method, write_method)\
   void update_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    return_type value\
   ) final;\
   void update_vector_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    size_t size,\
    const type *value\
   ) final;
   #include "joedb/TYPE_MACRO.h"

   bool wants_blob_data() const final {return true;}
   Blob write_blob_data(const std::string &data) final;

   Async_Writer get_async_tail_writer()
   {
    return Async_Writer(file, get_checkpoint_position());
   }

   Writable_Journal *get_writable_journal() override {return this;}

   void lock_pull();
   bool is_locked() const {return file.tail_is_locked();}
   void unlock() noexcept;

   ~Writable_Journal() override;
 };

 class Journal_Lock
 {
  private:
   Writable_Journal &journal;

  public:
   Journal_Lock(Writable_Journal &journal): journal(journal)
   {
    if (journal.get_position() > journal.get_checkpoint_position())
     throw Exception("locking journal with uncheckpointed data");
    journal.lock_pull();
   }

   Journal_Lock(const Journal_Lock &lock) = delete;
   Journal_Lock &operator=(const Journal_Lock &lock) = delete;

   ~Journal_Lock()
   {
    journal.unlock();
   }
 };
}

#endif
