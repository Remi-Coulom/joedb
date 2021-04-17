#ifndef joedb_Writable_Journal_declared
#define joedb_Writable_Journal_declared

#include "joedb/Writable.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/Async_Writer.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Writable_Journal: public Readonly_Journal, public Writable
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   int current_commit_level;

  public:
   Writable_Journal(Generic_File &file);

   //////////////////////////////////////////////////////////////////////////
   class Tail_Writer
   //////////////////////////////////////////////////////////////////////////
   {
    private:
     Writable_Journal &journal;
     const int64_t old_checkpoint;
     Async_Writer writer;

     Tail_Writer(const Tail_Writer &) = delete;

    public:
     Tail_Writer(Writable_Journal &journal):
      journal(journal),
      old_checkpoint(journal.get_checkpoint_position()),
      writer(journal.file, old_checkpoint)
     {
     }

     void append(const char *buffer, size_t size)
     {
      writer.write(buffer, size);
     }

     ~Tail_Writer()
     {
      journal.checkpoint(0);
      journal.file.set_position(old_checkpoint);
     }
   };

   void append_raw_tail(const std::vector<char> &data);

   int64_t ahead_of_checkpoint() const;
   void checkpoint(int commit_level);

   void create_table(const std::string &name) override;
   void drop_table(Table_Id table_id) override;
   void rename_table(Table_Id table_id,
                     const std::string &name) override;
   void add_field(Table_Id table_id,
                  const std::string &name,
                  Type type) override;
   void drop_field(Table_Id table_id,
                   Field_Id field_id) override;
   void rename_field(Table_Id table_id,
                     Field_Id field_id,
                     const std::string &name) override;
   void custom(const std::string &name) override;
   void comment(const std::string &comment) override;
   void timestamp(int64_t timestamp) override;
   void valid_data() override;
   void insert_into(Table_Id table_id, Record_Id record_id) override;
   void insert_vector(Table_Id table_id,
                      Record_Id record_id,
                      Record_Id size) override;
   void delete_from(Table_Id table_id, Record_Id record_id) override;

   #define TYPE_MACRO(type, return_type, type_id, read_method, write_method)\
   void update_##type_id(Table_Id table_id,\
                         Record_Id record_id,\
                         Field_Id field_id,\
                         return_type value) override;\
   void update_vector_##type_id(Table_Id table_id,\
                                Record_Id record_id,\
                                Field_Id field_id,\
                                Record_Id size,\
                                const type *value) override;
   #include "joedb/TYPE_MACRO.h"

   ~Writable_Journal() override;
 };
}

#endif
