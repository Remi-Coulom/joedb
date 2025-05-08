#ifndef joedb_Multiplexer_declared
#define joedb_Multiplexer_declared

#include <vector>
#include <initializer_list>
#include <functional>

#include "joedb/Writable.h"

namespace joedb
{
 /// @ingroup joedb
 class Multiplexer: public virtual Writable
 {
  private:
   const std::vector<std::reference_wrapper<Writable>> writables;
   size_t start_index;

  public:
   Multiplexer
   (
    std::initializer_list<std::reference_wrapper<Writable>> initializer_list
   );

   void set_start_index(size_t index) {start_index = index;}

   void create_table(const std::string &name) override;
   void drop_table(Table_Id table_id) override;
   void rename_table(Table_Id table_id, const std::string &name) override;
   void add_field
   (
    Table_Id table_id,
    const std::string &name,
    Type type
   ) override;
   void drop_field(Table_Id table_id, Field_Id field_id) override;
   void rename_field
   (
    Table_Id table_id,
    Field_Id field_id,
    const std::string &name
   ) override;
   void custom(const std::string &name) override;
   void comment(const std::string &comment) override;
   void timestamp(int64_t timestamp) override;
   void valid_data() override;
   void flush() override;

   int64_t get_position() const override;
   void start_writing(int64_t position) override;
   void end_writing(int64_t position) override;
   void soft_checkpoint() override;
   void hard_checkpoint() override;

   void insert_into(Table_Id table_id, Record_Id record_id) override;
   void delete_from(Table_Id table_id, Record_Id record_id) override;

   void insert_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    size_t size
   ) override;

   void delete_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    size_t size
   ) override;

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    return_type value\
   ) override;\
   void update_vector_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    size_t size,\
    const type *value\
   ) override;\
   type *get_own_##type_id##_storage\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    size_t &capacity\
   ) override;
   #include "joedb/TYPE_MACRO.h"

   void on_blob(Blob blob) override;
   bool wants_blob_data() const override;
   Blob write_blob(const std::string &data) override;

   ~Multiplexer() override;
 };
}

#endif
