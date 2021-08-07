#ifndef joedb_Writable_declared
#define joedb_Writable_declared

#include "Type.h"

#include <string>

namespace joedb
{
 class Writable
 {
  public:
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

   virtual void insert_into(Table_Id table_id, Record_Id record_id) {}
   virtual void insert_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    Record_Id size
   ) {}
   virtual void delete_from(Table_Id table_id, Record_Id record_id) {}

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   virtual void update_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    return_type value\
   ) {}\
   virtual void update_vector_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    Record_Id size,\
    const type *value\
   );\
   virtual type *get_own_##type_id##_storage\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    Record_Id &capacity\
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
    Record_Id &capacity\
   ) const\
   {\
    return ((Writable *)(this))->get_own_##type_id##_storage(table_id, record_id, field_id, capacity);\
   }
   #include "joedb/TYPE_MACRO.h"

   virtual ~Writable() noexcept(false) {}
 };
}

#endif
