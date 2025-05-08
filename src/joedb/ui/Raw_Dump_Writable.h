#ifndef joedb_Raw_Dump_Writable_declared
#define joedb_Raw_Dump_Writable_declared

#include "joedb/Writable.h"

#include <iostream>

namespace joedb
{
 /// @ingroup ui
 class Raw_Dump_Writable: public Writable
 {
  private:
   std::ostream &out;

   void write_type(Type type);

  public:
   Raw_Dump_Writable(std::ostream &out): out(out) {}

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
   ) override;
   #include "joedb/TYPE_MACRO.h"

   bool wants_blob_data() const override {return true;}
   Blob write_blob(const std::string &data) override;

   ~Raw_Dump_Writable();
 };
}

#endif
