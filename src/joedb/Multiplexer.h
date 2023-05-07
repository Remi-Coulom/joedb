#ifndef joedb_Multiplexer_declared
#define joedb_Multiplexer_declared

#include <vector>
#include <initializer_list>
#include <functional>

#include "Writable.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Multiplexer: public virtual Writable
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::vector<std::reference_wrapper<Writable>> writables;

  public:
   Multiplexer
   (
    std::initializer_list<std::reference_wrapper<Writable>> initializer_list
   );
   void add_writable(Writable &writable);

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
   void checkpoint(Commit_Level commit_level) final;
   void insert_into(Table_Id table_id, Record_Id record_id) final;
   void insert_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    Record_Id size
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
    Record_Id size,\
    const type *value\
   ) final;\
   type *get_own_##type_id##_storage\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    Record_Id &capacity\
   ) final;
   #include "joedb/TYPE_MACRO.h"

   bool wants_blobs() const final;
   void on_blob(Blob blob, Blob_Reader &reader) final;
   Blob write_blob_data(const std::string &data) final;

   ~Multiplexer() override;
 };
}

#endif
