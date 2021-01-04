#ifndef joedb_Database_declared
#define joedb_Database_declared

#include "joedb/Readable_Writable.h"
#include "joedb/interpreter/Table.h"

#include <map>

namespace joedb
{
 class Database: public Readable_Writable
 {
  private:
   const Record_Id max_record_id;

   std::map<Table_Id, Table> tables;
   std::map<Table_Id, std::string> table_names;
   Table_Id current_table_id = 0;

   void check_identifier(const char *message, const std::string &name) const;

  public:
   Database(Record_Id max_record_id = 0): max_record_id(max_record_id) {}

   //
   // Readable override
   //
   const std::map<Table_Id, std::string> &get_tables() const override {return table_names;}
   const std::map<Field_Id, std::string> &get_fields(Table_Id table_id) const override;
   const Type &get_field_type(Table_Id table_id,
                              Field_Id field_id) const override;

   Record_Id get_last_record_id(Table_Id table_id) const override;
   bool is_used(Table_Id table_id, Record_Id record_id) const override;
   const Compact_Freedom_Keeper &get_freedom(Table_Id table_id) const override;
   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   return_type get_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id\
   ) const override;\
   const type &get_##type_id##_storage\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id\
   ) const override;
   #include "joedb/TYPE_MACRO.h"

   //
   // Writable override
   //
   void create_table(const std::string &name) override;
   void drop_table(Table_Id table_id) override;
   void rename_table(Table_Id table_id, const std::string &name) override;
   void add_field(Table_Id table_id,
                  const std::string &name,
                  Type type) override;
   void drop_field(Table_Id table_id, Field_Id field_id) override;
   void rename_field(Table_Id, Field_Id, const std::string &name) override;

   void custom(const std::string &name) override {};
   void comment(const std::string &comment) override {};
   void timestamp(int64_t timestamp) override {};
   void valid_data() override {};

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
    Field_Id field_id,\
    Record_Id &capacity\
   ) override;
   #include "joedb/TYPE_MACRO.h"
 };
}

#endif
