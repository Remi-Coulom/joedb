#ifndef joedb_Interpreter_Dump_Writable_declared
#define joedb_Interpreter_Dump_Writable_declared

#include "joedb/interpreted/Database_Schema.h"
#include "joedb/Multiplexer.h"

namespace joedb
{
 /// @ingroup ui
 class Interpreter_Writable: public Writable
 {
  private:
   std::ostream &out;
   const Database_Schema &schema;
   const bool blob_wanted;

   void write_type(Type type);

  public:
   Interpreter_Writable
   (
    std::ostream &out,
    const Database_Schema &schema,
    bool blob_wanted = true
   ):
    out(out),
    schema(schema),
    blob_wanted(blob_wanted)
   {
   }

   const char *get_name() const {return "dump";}

   void soft_checkpoint() override;

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

   void insert_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    size_t size
   ) override;

   void delete_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    size_t size
   ) override;

   void write_update(const char *command, Table_Id table_id, Record_Id record_id, Field_Id field_id);

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
    size_t size,\
    const type *value\
   ) override;
   #include "joedb/TYPE_MACRO.h"

   bool wants_blob_data() const override {return blob_wanted;}
   void on_blob(Blob blob) override;
   Blob write_blob(const std::string &data) override;

   ~Interpreter_Writable();
 };

 class Interpreter_Dump_Writable_Parent
 {
  protected:
   Database_Schema schema;
   Interpreter_Writable interpreter_writable;

  public:
   Interpreter_Dump_Writable_Parent(std::ostream &out, bool blob_wanted):
    interpreter_writable(out, schema, blob_wanted)
   {
   }
 };

 class Interpreter_Dump_Writable:
  public Interpreter_Dump_Writable_Parent,
  public Multiplexer
 {

  public:
   Interpreter_Dump_Writable(std::ostream &out, bool blob_wanted = true):
    Interpreter_Dump_Writable_Parent(out, blob_wanted),
    Multiplexer{interpreter_writable, schema}
   {
   }
 };
}

#endif
