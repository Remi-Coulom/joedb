#ifndef joedb_Writable_declared
#define joedb_Writable_declared

#include "joedb/Type.h"

#include <string>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 enum class Commit_Level
 ////////////////////////////////////////////////////////////////////////////
 {
  no_commit,
  half_commit,
  full_commit
 };

 ////////////////////////////////////////////////////////////////////////////
 class Writable: public Blob_Writer
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   const Commit_Level default_commit_level;

  public:
   Writable(Commit_Level default_commit_level = Commit_Level::no_commit):
    default_commit_level(default_commit_level)
   {
   }

   virtual void create_table(const std::string &name) {}
   virtual void drop_table(Table_Id table_id) {}
   virtual void rename_table(Table_Id table_id, const std::string &name) {}

   virtual void add_field
   (
    Table_Id table_id,
    const std::string &name,
    Type type
   ) {}
   virtual void drop_field(Table_Id table_id, Field_Id field_id) {}
   virtual void rename_field
   (
    Table_Id table_id,
    Field_Id field_id,
    const std::string &name
   ) {}

   virtual void custom(const std::string &name) {}
   virtual void comment(const std::string &comment) {}
   virtual void timestamp(int64_t timestamp) {}
   virtual void valid_data() {}
   virtual void flush() {}
   virtual void checkpoint(Commit_Level commit_level) {}

   Commit_Level get_default_commit_level() const {return default_commit_level;}
   void default_checkpoint();

   virtual void insert_into(Table_Id table_id, Record_Id record_id) {}
   virtual void insert_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    size_t size
   ) {}
   virtual void delete_from(Table_Id table_id, Record_Id record_id) {}

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   virtual void update_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    return_type value\
   );
   #include "joedb/TYPE_MACRO.h"

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   virtual void update_vector_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    size_t size,\
    const type *value\
   );\
   virtual type *get_own_##type_id##_storage\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    size_t &capacity\
   )\
   {\
    capacity = 0;\
    return nullptr;\
   }\
   const type *get_own_##type_id##_const_storage\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    size_t &capacity\
   ) const\
   {\
    return (const_cast<Writable *>(this))->get_own_##type_id##_storage(table_id, record_id, field_id, capacity);\
   }
   #include "joedb/TYPE_MACRO.h"

   virtual bool wants_blobs() const {return false;}
   virtual void on_blob(Blob blob, Blob_Reader &reader) {}

   virtual ~Writable() = default;
 };
}

#endif
