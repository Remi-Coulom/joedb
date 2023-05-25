#ifndef joedb_Writable_Journal_declared
#define joedb_Writable_Journal_declared

#include "joedb/Writable.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/Async_Writer.h"
#include "joedb/Posthumous_Thrower.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Writable_Journal: 
 ////////////////////////////////////////////////////////////////////////////
  public Readonly_Journal,
  public Writable,
  public Posthumous_Thrower
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

  public:
   explicit Writable_Journal(Generic_File &file);

   //////////////////////////////////////////////////////////////////////////
   class Tail_Writer
   //////////////////////////////////////////////////////////////////////////
   {
    private:
     Writable_Journal &journal;
     const int64_t old_position;
     Async_Writer writer;

     Tail_Writer(const Tail_Writer &) = delete;

    public:
     Tail_Writer(Writable_Journal &journal):
      journal(journal),
      old_position(journal.get_position()),
      writer(journal.file, journal.get_checkpoint_position())
     {
     }

     void append(const char *buffer, size_t size)
     {
      writer.write(buffer, size);
     }

     void finish()
     {
      writer.seek();
      journal.checkpoint(Commit_Level::no_commit);
      journal.file.set_position(old_position);
     }
   };

   void append() {file.set_position(checkpoint_position);}
   void append_raw_tail(const char *data, size_t size);
   void append_raw_tail(const std::vector<char> &data);

   template<typename F> void exclusive_transaction(F transaction)
   {
    file.exclusive_transaction(transaction);
   }

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
    Record_Id size
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
    Record_Id size,\
    const type *value\
   ) final;
   #include "joedb/TYPE_MACRO.h"

   bool wants_blobs() const final {return true;}
   Blob write_blob_data(const std::string &data) final;

   ~Writable_Journal() override;
 };
}

#endif
