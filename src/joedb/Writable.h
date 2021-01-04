#ifndef joedb_Writable_declared
#define joedb_Writable_declared

#include "Type.h"

#include <string>

namespace joedb
{
 class Writable
 {
  public:
   virtual void create_table(const std::string &name) = 0;
   virtual void drop_table(Table_Id table_id) = 0;
   virtual void rename_table(Table_Id table_id, const std::string &name) = 0;

   virtual void add_field(Table_Id table_id,
                          const std::string &name,
                          Type type) = 0;
   virtual void drop_field(Table_Id table_id, Field_Id field_id) = 0;
   virtual void rename_field(Table_Id table_id,
                             Field_Id field_id,
                             const std::string &name) = 0;

   virtual void custom(const std::string &name) = 0;
   virtual void comment(const std::string &comment) = 0;
   virtual void timestamp(int64_t timestamp) = 0;
   virtual void valid_data() = 0;

   virtual void insert_into(Table_Id table_id, Record_Id record_id) = 0;
   virtual void insert_vector(Table_Id table_id,
                              Record_Id record_id,
                              Record_Id size) = 0;
   virtual void delete_from(Table_Id table_id, Record_Id record_id) = 0;

   virtual Record_Id get_storage_capacity(Table_Id table_id) const {return 0;}

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
    Field_Id field_id\
   )\
   {\
    return nullptr;\
   }
   #include "joedb/TYPE_MACRO.h"

   virtual ~Writable() {}
 };
}

#endif
