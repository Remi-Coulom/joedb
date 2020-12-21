#ifndef joedb_Raw_Dump_Writable_declared
#define joedb_Raw_Dump_Writable_declared

#include "joedb/io/Dump_Writable.h"
#include "joedb/io/type_io.h"

namespace joedb
{
 class Raw_Dump_Writable: public Writable
 {
  private:
   std::ostream &out;

   void write_type(Type type)
   {
    switch(type.get_type_id())
    {
     case Type::Type_Id::null:
      out << "null";
     break;

     #define TYPE_MACRO(type, return_type, type_id, read, write)\
     case Type::Type_Id::type_id:\
      out << #type_id;\
     break;
     #define TYPE_MACRO_NO_REFERENCE
     #include "joedb/TYPE_MACRO.h"

     case Type::Type_Id::reference:
      out << "references " << type.get_table_id();
     break;
    }
   }

  public:
   Raw_Dump_Writable(std::ostream &out): out(out) {}

   void create_table(const std::string &name) override
   {
    out << "create_table " << name << '\n';
   }

   void drop_table(Table_Id table_id) override
   {
    out << "drop_table " << table_id << '\n';
   }

   void rename_table(Table_Id table_id,
                     const std::string &name) override
   {
    out << "rename_table " << table_id << ' ' << name << '\n';
   }

   void add_field(Table_Id table_id,
                  const std::string &name,
                  Type type) override
   {
    out << "add_field " << table_id << ' ' << name << ' ';
    write_type(type);
    out << '\n';
   }

   void drop_field(Table_Id table_id, Field_Id field_id) override
   {
    out << "drop_field " << table_id << ' ' << field_id << '\n';
   }

   void rename_field(Table_Id table_id,
                     Field_Id field_id,
                     const std::string &name) override
   {
    out << "rename_field " << table_id << ' ';
    out << field_id << ' ' << name << '\n';
   }

   void custom(const std::string &name) override
   {
    out << "custom " << name << '\n';
   }

   void comment(const std::string &comment) override
   {
    out << "comment ";
    write_string(out, comment);
    out << '\n';
   }

   void timestamp(int64_t timestamp) override
   {
    out << "timestamp " << timestamp << '\n';
   }

   void valid_data() override
   {
    out << "valid_data\n";
   }

   void insert_into(Table_Id table_id, Record_Id record_id) override
   {
    out << "insert_into " << table_id << ' ' << record_id << '\n';
   }

   void insert_vector(Table_Id table_id,
                      Record_Id record_id,
                      Record_Id size) override
   {
    out << "insert_vector " << table_id << ' ';
    out << record_id << ' ' << size << '\n';
   }

   void delete_from(Table_Id table_id, Record_Id record_id) override
   {
    out << "delete_from " << table_id << ' ';
    out << record_id << '\n';
   }

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id(Table_Id table_id,\
                         Record_Id record_id,\
                         Field_Id field_id,\
                         return_type value) override\
   {\
    out << "update_" << #type_id << ' ' << table_id << ' ';\
    out << record_id << ' ';\
    out << field_id << ' ';\
    joedb::write_##type_id(out, value);\
    out << '\n';\
   }\
   void update_vector_##type_id(Table_Id table_id,\
                                Record_Id record_id,\
                                Field_Id field_id,\
                                Record_Id size,\
                                const type *value) override\
   {\
    out << "update_vector_" << #type_id << ' ' << table_id << ' ';\
    out << record_id << ' ';\
    out << field_id << ' ';\
    out << size;\
    for (Record_Id i = 0; i < size; i++)\
    {\
     out << ' ';\
     joedb::write_##type_id(out, value[i]);\
    }\
    out << '\n';\
   }
   #include "joedb/TYPE_MACRO.h"
 };
}

#endif
