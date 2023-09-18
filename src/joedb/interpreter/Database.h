#ifndef joedb_Database_declared
#define joedb_Database_declared

#include "joedb/interpreter/Database_Schema.h"

namespace joedb
{
 ///////////////////////////////////////////////////////////////////////////
 class Database: public Database_Schema
 ///////////////////////////////////////////////////////////////////////////
 {
  private:
   const Size max_record_id;

  public:
   Database(Size max_record_id = 0):
    max_record_id(max_record_id)
   {
   }

   void insert_into(Table_Id table_id, Record_Id record_id) final;
   void insert_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    Size size
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
    Size size,\
    const type *value\
   ) final;\
   type *get_own_##type_id##_storage\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    Size &capacity\
   ) final;
   #include "joedb/TYPE_MACRO.h"

   ~Database();
 };
}

#endif
