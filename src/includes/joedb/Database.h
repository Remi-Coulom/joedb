#ifndef joedb_Database_declared
#define joedb_Database_declared

#include "Table.h"
#include "Dummy_Listener.h"

#include <map>

namespace joedb
{
 class Database
 {
  private:
   table_id_t current_table_id = 0;

   std::map<table_id_t, Table> tables;

   Dummy_Listener dummy_listener;
   Listener *listener = &dummy_listener;

   std::vector<std::string> custom_names;

  public:
   void set_listener(Listener &new_listener) {listener = &new_listener;}
   void clear_listener() {listener = &dummy_listener;}

   const std::map<table_id_t, Table> &get_tables() const {return tables;}
   size_t get_current_table_id() const {return current_table_id;}

   table_id_t create_table(const std::string &name);
   bool drop_table(table_id_t table_id);
   bool rename_table(table_id_t table_id, const std::string &name);
   table_id_t find_table(const std::string &name) const;
   field_id_t add_field(table_id_t table_id,
                        const std::string &name,
                        Type type);
   field_id_t find_field(table_id_t table_id,
                         const std::string &name) const;
   Type::type_id_t get_field_type(table_id_t table_id,
                                  field_id_t field_id) const;
   bool drop_field(table_id_t table_id, field_id_t field_id);
   bool rename_field(table_id_t, field_id_t, const std::string &name);
   void custom(const std::string &name);
   void comment(const std::string &comment) const;
   void time_stamp(int64_t time_stamp) const;
   void checkpoint() const;
   const std::vector<std::string> &get_custom_names() const
   {
    return custom_names;
   }
   bool insert_into(table_id_t table_id, record_id_t record_id);
   bool insert_vector(table_id_t table_id,
                      record_id_t record_id,
                      record_id_t size);
   bool delete_from(table_id_t table_id, record_id_t record_id);

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   bool update_##type_id(table_id_t table_id,\
                         record_id_t record_id,\
                         field_id_t field_id,\
                         return_type value);\
   bool update_vector_##type_id(table_id_t table_id,\
                                record_id_t record_id,\
                                field_id_t field_id,\
                                record_id_t size,\
                                const type *value);
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO
 };
}

#endif
