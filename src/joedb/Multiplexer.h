#ifndef joedb_Multiplexer_declared
#define joedb_Multiplexer_declared

#include <vector>

#include "Writable.h"

namespace joedb
{
 class Multiplexer: public virtual Writable
 {
  private:
   std::vector<Writable *> writables;

  public:
   void add_writable(Writable &writable);

   void create_table(const std::string &name) override;
   void drop_table(Table_Id table_id) override;
   void rename_table(Table_Id table_id, const std::string &name) override;
   void add_field(Table_Id table_id,
                  const std::string &name,
                  Type type) override;
   void drop_field(Table_Id table_id, Field_Id field_id) override;
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
    Record_Id size,\
    const type *value\
   ) override;\
   type *get_own_##type_id##_storage\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id\
   ) override;
   #include "joedb/TYPE_MACRO.h"
 };
}

#endif
