#ifndef joedb_Interpreter_Dump_Writable_declared
#define joedb_Interpreter_Dump_Writable_declared

#include "joedb/interpreter/Database.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Interpreter_Dump_Writable: public Writable
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::ostream &out;
   const bool blob_wanted;
   Database_Schema schema;
   bool muted;

   void write_type(Type type);

  public:
   Interpreter_Dump_Writable(std::ostream &out, bool blob_wanted = false):
    out(out),
    blob_wanted(blob_wanted),
    muted(false)
   {
   }

   void set_muted(bool new_muted)
   {
    muted = new_muted;
   }

   void create_table(const std::string &name) final;
   void drop_table(Table_Id table_id) final;
   void rename_table(Table_Id table_id, const std::string &name) final;
   void add_field
   (
    Table_Id table_id,
    const std::string &name,
    Type type
   ) final;
   void drop_field(Table_Id table_id, Field_Id field_id) final;
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

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
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

   bool wants_blobs() const final {return blob_wanted;}
   void on_blob(Blob blob, Blob_Reader &reader) final;
   Blob write_blob_data(const std::string &data) final;

   ~Interpreter_Dump_Writable();
 };
}

#endif
