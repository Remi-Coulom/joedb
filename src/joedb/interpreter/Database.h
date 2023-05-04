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
   const bool blob_by_value;

  public:
   Database(Record_Id max_record_id = 0, bool blob_by_value = true):
    max_record_id(max_record_id),
    blob_by_value(blob_by_value)
   {
   }

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

   bool wants_blob_by_value() override {return blob_by_value;}

   Blob update_blob_value
   (
    Table_Id table_id,
    Record_Id record_id,
    Field_Id field_id,
    const std::string &value
   ) override;

   ~Database();
 };
}

#endif
