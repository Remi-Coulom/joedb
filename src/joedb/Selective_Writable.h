#ifndef joedb_Selective_Writable_declared
#define joedb_Selective_Writable_declared

#include "joedb/Writable.h"

namespace joedb
{
 /// @ingroup joedb
 class Selective_Writable: public Writable
 {
  public:
   enum Mode
   {
    schema = 1,
    data = 2,
    information = 4,

    data_and_schema = 3, // bitwise operators don't work on enums
    data_and_information = 6,
    all = 7
   };

  private:
   Writable &writable;
   const Mode mode;
   bool blob_found;

  public:

   Selective_Writable(Writable &writable, Mode mode);

   bool has_blobs() const {return blob_found;}

   //
   // schema events
   //
   void create_table(const std::string &name) override;
   void drop_table(Table_Id table_id) override;
   void rename_table(Table_Id table_id, const std::string &name) override;
   void add_field(Table_Id table_id, const std::string &name, Type type) override;
   void drop_field(Table_Id table_id, Field_Id field_id) override;
   void rename_field(Table_Id table_id, Field_Id field_id, const std::string &name) override;
   void custom(const std::string &name) override;

   //
   // Informative events
   //
   void comment(const std::string &comment) override;
   void timestamp(int64_t timestamp) override;
   void valid_data() override;
   int64_t get_position() const override;
   void start_writing(int64_t position) override;
   void soft_checkpoint_at(int64_t position) override;
   void hard_checkpoint_at(int64_t position) override;

   //
   // data events
   //
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

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id(Table_Id table_id,\
                         Record_Id record_id,\
                         Field_Id field_id,\
                         return_type value) override;
   #include "joedb/TYPE_MACRO.h"

   void on_blob(Blob blob) final;
 };
}

#endif
