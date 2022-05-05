#ifndef joedb_SQL_Dump_Writable_declared
#define joedb_SQL_Dump_Writable_declared

#include "joedb/interpreter/Database.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class SQL_Dump_Writable: public Writable
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::ostream &out;
   Database_Schema schema;
   const bool drop_column;
   void write_type(Type type);
   std::string id_field_name = "\"__id\"";
   std::string key_type = "INTEGER";

  public:
   SQL_Dump_Writable(std::ostream &out, bool drop_column = true):
    out(out),
    drop_column(drop_column)
   {}

   void create_table(const std::string &name) final;
   void drop_table(Table_Id table_id) final;
   void rename_table(Table_Id table_id, const std::string &name) final;
   void add_field
   (
    Table_Id table_id,
    const std::string &name,
    Type type
   ) final;
   void drop_field(Table_Id table_id, Field_Id field_id) final;
   void rename_field
   (
    Table_Id table_id,
    Field_Id field_id,
    const std::string &name
   ) final;
   void custom(const std::string &name) final;
   void comment(const std::string &comment) final;
   void timestamp(int64_t timestamp) final;
   void valid_data() final;
   void insert_into(Table_Id table_id, Record_Id record_id) final;
   void insert_vector(Table_Id table_id,
                      Record_Id record_id,
                      Record_Id size) final;
   void delete_from(Table_Id table_id, Record_Id record_id) final;

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id(Table_Id table_id,\
                         Record_Id record_id,\
                         Field_Id field_id,\
                         return_type value) final;
   #include "joedb/TYPE_MACRO.h"

   ~SQL_Dump_Writable();
 };
}

#endif
