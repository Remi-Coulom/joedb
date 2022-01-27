#ifndef joedb_SQL_Dump_Writable_declared
#define joedb_SQL_Dump_Writable_declared

#include "joedb/interpreter/Database.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class SQL_Dump_Writable: public Database_Schema
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::ostream &out;
   const bool drop_column;
   void write_type(Type type);
   std::string id_field_name = "\"__id\"";
   std::string key_type = "INTEGER";

  public:
   SQL_Dump_Writable(std::ostream &out, bool drop_column = true):
    out(out),
    drop_column(drop_column)
   {}

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
   void insert_vector(Table_Id table_id,
                      Record_Id record_id,
                      Record_Id size) override;
   void delete_from(Table_Id table_id, Record_Id record_id) override;

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id(Table_Id table_id,\
                         Record_Id record_id,\
                         Field_Id field_id,\
                         return_type value) override;
   #include "joedb/TYPE_MACRO.h"
 };
}

#endif
