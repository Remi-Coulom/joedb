#ifndef joedb_Interpreter_Dump_Writeable_declared
#define joedb_Interpreter_Dump_Writeable_declared

#include "Dump_Writeable.h"
#include "type_io.h"

namespace joedb
{
 class Interpreter_Dump_Writeable: public Dump_Writeable
 {
  private:
   void write_type(Type type)
   {
    switch(type.get_type_id())
    {
     case Type::type_id_t::null:
      out << "null";
     break;

     #define TYPE_MACRO(type, return_type, type_id, read, write)\
     case Type::type_id_t::type_id:\
      out << #type_id;\
     break;
     #define TYPE_MACRO_NO_REFERENCE
     #include "joedb/TYPE_MACRO.h"
     #undef TYPE_MACRO_NO_REFERENCE
     #undef TYPE_MACRO

     case Type::type_id_t::reference:
     {
      out << "references ";
      table_id_t table_id = type.get_table_id();
      const auto it = db.get_tables().find(table_id);
      if (it != db.get_tables().end())
       out << it->second.get_name();
      else
       out << "a_deleted_table";
     }
     break;
    }
   }

  public:
   Interpreter_Dump_Writeable(std::ostream &out): Dump_Writeable(out) {}

   void create_table(const std::string &name) override
   {
    out << "create_table " << name << '\n';
    Schema_Writeable::create_table(name);
   }

   void drop_table(table_id_t table_id) override
   {
    out << "drop_table " << get_table_name(table_id) << '\n';
    Schema_Writeable::drop_table(table_id);
   }

   void rename_table(table_id_t table_id,
                     const std::string &name) override
   {
    out << "rename_table " << get_table_name(table_id) << ' ' << name << '\n';
    Schema_Writeable::rename_table(table_id, name);
   }

   void add_field(table_id_t table_id,
                  const std::string &name,
                  Type type) override
   {
    out << "add_field " << get_table_name(table_id) << ' ' << name << ' ';
    write_type(type);
    out << '\n';
    Schema_Writeable::add_field(table_id, name, type);
   }

   void drop_field(table_id_t table_id, field_id_t field_id) override
   {
    out << "drop_field " << get_table_name(table_id) << ' ';
    out << get_field_name(table_id, field_id) << '\n';
    Schema_Writeable::drop_field(table_id, field_id);
   }

   void rename_field(table_id_t table_id,
                     field_id_t field_id,
                     const std::string &name) override
   {
    out << "rename_field " << get_table_name(table_id) << ' ';
    out << get_field_name(table_id, field_id) << ' ' << name << '\n';
    Schema_Writeable::rename_field(table_id, field_id, name);
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
    out << "timestamp " << timestamp << ' ';
    out << get_local_time(timestamp) << '\n';
   }

   void valid_data() override
   {
    out << "valid_data\n";
   }

   void insert(table_id_t table_id, record_id_t record_id) override
   {
    out << "insert_into " << get_table_name(table_id) << ' ';
    out << record_id << '\n';
   }

   void insert_vector(table_id_t table_id,
                      record_id_t record_id,
                      record_id_t size) override
   {
    out << "insert_vector " << get_table_name(table_id) << ' ';
    out << record_id << ' ' << size << '\n';
   }

   void delete_record(table_id_t table_id, record_id_t record_id) override
   {
    out << "delete_from " << get_table_name(table_id) << ' ';
    out << record_id << '\n';
   }

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id(table_id_t table_id,\
                         record_id_t record_id,\
                         field_id_t field_id,\
                         return_type value) override\
   {\
    out << "update " << get_table_name(table_id) << ' ';\
    out << record_id << ' ';\
    out << get_field_name(table_id, field_id) << ' ';\
    joedb::write_##type_id(out, value);\
    out << '\n';\
   }\
   void update_vector_##type_id(table_id_t table_id,\
                                record_id_t record_id,\
                                field_id_t field_id,\
                                record_id_t size,\
                                const type *value) override\
   {\
    out << "update_vector " << get_table_name(table_id) << ' ';\
    out << record_id << ' ';\
    out << get_field_name(table_id, field_id) << ' ';\
    out << size;\
    for (record_id_t i = 0; i < size; i++)\
    {\
     out << ' ';\
     joedb::write_##type_id(out, value[i]);\
    }\
    out << '\n';\
   }
   #include "joedb/TYPE_MACRO.h"
   #undef TYPE_MACRO
 };
}

#endif
