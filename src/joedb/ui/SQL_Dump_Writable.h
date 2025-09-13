#ifndef joedb_SQL_Dump_Writable_declared
#define joedb_SQL_Dump_Writable_declared

#include "joedb/interpreted/Database_Schema.h"
#include "joedb/Multiplexer.h"

namespace joedb
{
 class Abstract_File;

 /// @ingroup ui
 class SQL_Writable: public Loop_Writable
 {
  private:
   static constexpr const char *id_field_name = "\"__id\"";
   static constexpr const char *key_type = "INTEGER";

   std::ostream &out;
   const Database_Schema &schema;
   const Abstract_File *blob_reader;

   void write_type(Type type);
   void write_update(Table_Id table_id, Field_Id field_id);
   void write_where(Record_Id record_id);

  public:
   SQL_Writable
   (
    std::ostream &out,
    const Database_Schema &schema,
    const Abstract_File *blob_reader = nullptr
   ):
    out(out),
    schema(schema),
    blob_reader(blob_reader)
   {}

   const char *get_name() const {return "sql";}

   void start_writing(int64_t position) override;
   void end_writing(int64_t position) override;

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
   void delete_from(Table_Id table_id, Record_Id record_id) override;

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id(Table_Id table_id,\
                         Record_Id record_id,\
                         Field_Id field_id,\
                         return_type value) override;
   #include "joedb/TYPE_MACRO.h"

   ~SQL_Writable();
 };

 class SQL_Dump_Writable_Parent
 {
  protected:
   Database_Schema schema;
   SQL_Writable interpreter_writable;

  public:
   SQL_Dump_Writable_Parent
   (
    std::ostream &out,
    const Abstract_File *blob_reader = nullptr
   ):
    interpreter_writable(out, schema, blob_reader)
   {
   }
 };

 class SQL_Dump_Writable:
  public SQL_Dump_Writable_Parent,
  public Multiplexer
 {
  public:
   SQL_Dump_Writable
   (
    std::ostream &out,
    const Abstract_File *blob_reader = nullptr
   ):
    SQL_Dump_Writable_Parent(out, blob_reader),
    Multiplexer{interpreter_writable, schema}
   {
   }
 };
}

#endif
