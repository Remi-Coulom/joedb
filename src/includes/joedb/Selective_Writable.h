#ifndef joedb_Selective_Writable_declared
#define joedb_Selective_Writable_declared

#include "Writable.h"

namespace joedb
{
 class Selective_Writable: public Writable
 {
  public:
   enum Mode
   {
    schema = 1,
    data = 2,
    information = 4,

    data_and_schema = 3, // bitwise operators don't work on enums
    all = 7
   };

  private:
   Writable &writable;
   const Mode mode;

  public:

   Selective_Writable(Writable &writable, Mode mode):
    writable(writable),
    mode(mode)
   {
   }

   //
   // schema events
   //
   void create_table(const std::string &name) override
   {
    if (mode & schema)
     writable.create_table(name);
   }

   void drop_table(Table_Id table_id) override
   {
    if (mode & schema)
     writable.drop_table(table_id);
   }

   void rename_table(Table_Id table_id,
                     const std::string &name) override
   {
    if (mode & schema)
     writable.rename_table(table_id, name);
   }

   void add_field(Table_Id table_id,
                  const std::string &name,
                  Type type) override
   {
    if (mode & schema)
     writable.add_field(table_id, name, type);
   }

   void drop_field(Table_Id table_id,
                   Field_Id field_id) override
   {
    if (mode & schema)
     writable.drop_field(table_id, field_id);
   }

   void rename_field(Table_Id table_id,
                     Field_Id field_id,
                     const std::string &name) override
   {
    if (mode & schema)
     writable.rename_field(table_id, field_id, name);
   }

   void custom(const std::string &name) override
   {
    if (mode & schema)
     writable.custom(name);
   }

   //
   // Informative events
   //
   void comment(const std::string &comment) override
   {
    if (mode & information)
     writable.comment(comment);
   }

   void timestamp(int64_t timestamp) override
   {
    if (mode & information)
     writable.timestamp(timestamp);
   }

   void valid_data() override
   {
    if (mode & information)
     writable.valid_data();
   }

   //
   // data events
   //
   void insert_into(Table_Id table_id, Record_Id record_id) override
   {
    if (mode & data)
     writable.insert_into(table_id, record_id);
   }

   void insert_vector(Table_Id table_id,
                      Record_Id record_id,
                      Record_Id size) override
   {
    if (mode & data)
     writable.insert_vector(table_id, record_id, size);
   }

   void delete_from(Table_Id table_id, Record_Id record_id) override
   {
    if (mode & data)
     writable.delete_from(table_id, record_id);
   }

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id(Table_Id table_id,\
                         Record_Id record_id,\
                         Field_Id field_id,\
                         return_type value) override\
   {\
    if (mode & data)\
     writable.update_##type_id(table_id, record_id, field_id, value);\
   }
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO
 };
}

#endif
