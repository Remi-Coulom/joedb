#ifndef joedb_Raw_Dump_Writable_declared
#define joedb_Raw_Dump_Writable_declared

#include "joedb/Writable.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Raw_Dump_Writable: public Writable
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::ostream &out;

   void write_type(Type type);

  public:
   Raw_Dump_Writable(std::ostream &out): out(out) {}

   void create_table(const std::string &name) final override;
   void drop_table(Table_Id table_id) final override;
   void rename_table(Table_Id table_id, const std::string &name) final override;
   void add_field
   (
    Table_Id table_id,
    const std::string &name,
    Type type
   ) final override;
   void drop_field(Table_Id table_id, Field_Id field_id) final override;
   void rename_field
   (
    Table_Id table_id,
    Field_Id field_id,
    const std::string &name
   ) final override;
   void custom(const std::string &name) final override;
   void comment(const std::string &comment) final override;
   void timestamp(int64_t timestamp) final override;
   void valid_data() final override;
   void insert_into(Table_Id table_id, Record_Id record_id) final override;
   void insert_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    Record_Id size
   ) final override;
   void delete_from(Table_Id table_id, Record_Id record_id) final override;

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    return_type value\
   ) final override;\
   void update_vector_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    Record_Id size,\
    const type *value\
   ) final override;
   #include "joedb/TYPE_MACRO.h"
 };
}

#endif
