#ifndef joedb_Selective_Writeable_declared
#define joedb_Selective_Writeable_declared

#include "joedb/Writeable.h"

namespace joedb
{
 class Selective_Writeable: public Writeable
 {
  public:
   enum Mode
   {
    schema = 1,
    data = 2,
    information = 4,

    data_and_schema = 3 // bitwise operators don't work on enums
   };

  private:
   Writeable &writeable;
   const Mode mode;

  public:

   Selective_Writeable(Writeable &writeable, Mode mode):
    writeable(writeable),
    mode(mode)
   {
   }

   //
   // schema events
   //
   void create_table(const std::string &name) override
   {
    if (mode & schema)
     writeable.create_table(name);
   }

   void drop_table(table_id_t table_id) override
   {
    if (mode & schema)
     writeable.drop_table(table_id);
   }

   void rename_table(table_id_t table_id,
                     const std::string &name) override
   {
    if (mode & schema)
     writeable.rename_table(table_id, name);
   }

   void add_field(table_id_t table_id,
                  const std::string &name,
                  Type type) override
   {
    if (mode & schema)
     writeable.add_field(table_id, name, type);
   }

   void drop_field(table_id_t table_id,
                   field_id_t field_id) override
   {
    if (mode & schema)
     writeable.drop_field(table_id, field_id);
   }

   void rename_field(table_id_t table_id,
                     field_id_t field_id,
                     const std::string &name) override
   {
    if (mode & schema)
     writeable.rename_field(table_id, field_id, name);
   }

   void custom(const std::string &name) override
   {
    if (mode & schema)
     writeable.custom(name);
   }

   //
   // Informative events
   //
   void comment(const std::string &comment) override
   {
    if (mode & information)
     writeable.comment(comment);
   }

   void timestamp(int64_t timestamp) override
   {
    if (mode & information)
     writeable.timestamp(timestamp);
   }

   void valid_data() override
   {
    if (mode & information)
     writeable.valid_data();
   }

   //
   // data events
   //
   void insert(table_id_t table_id, record_id_t record_id) override
   {
    if (mode & data)
     writeable.insert(table_id, record_id);
   }

   void insert_vector(table_id_t table_id,
                      record_id_t record_id,
                      record_id_t size) override
   {
    if (mode & data)
     writeable.insert_vector(table_id, record_id, size);
   }

   void delete_record(table_id_t table_id, record_id_t record_id) override
   {
    if (mode & data)
     writeable.delete_record(table_id, record_id);
   }

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id(table_id_t table_id,\
                         record_id_t record_id,\
                         field_id_t field_id,\
                         return_type value) override\
   {\
    if (mode & data)\
     writeable.update_##type_id(table_id, record_id, field_id, value);\
   }
   #include "joedb/TYPE_MACRO.h"
   #undef TYPE_MACRO
 };
}

#endif
