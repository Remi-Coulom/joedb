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
   const Record_Id max_record_id;

  public:
   Database(Record_Id max_record_id = 0);

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
   ) final override;\
   type *get_own_##type_id##_storage\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    Record_Id &capacity\
   ) final override;
   #include "joedb/TYPE_MACRO.h"

   ~Database();
 };
}

#endif
