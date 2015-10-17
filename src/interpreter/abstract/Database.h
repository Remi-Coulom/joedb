#ifndef joedb_Database_declared
#define joedb_Database_declared

#include "Table.h"
#include "Listener.h"

#include <map>

namespace joedb
{
 class Database
 {
  private:
   table_id_t current_table_id;

   std::map<table_id_t, Table> tables;

   Listener default_listener;
   Listener *listener;

  public:
   Database(): current_table_id(0), listener(&default_listener) {}

   void set_listener(Listener &new_listener) {listener = &new_listener;}
   void clear_listener() {listener = &default_listener;}

   const std::map<table_id_t, Table> &get_tables() const {return tables;}

   table_id_t create_table(const std::string &name);
   bool drop_table(table_id_t table_id);
   table_id_t find_table(const std::string &name) const;
   field_id_t add_field(table_id_t table_id,
                        const std::string &name,
                        Type type);
   field_id_t find_field(table_id_t table_id,
                         const std::string &name) const;
   Type::type_id_t get_field_type(table_id_t table_id,
                                  field_id_t field_id) const;
   bool drop_field(table_id_t table_id, field_id_t field_id);
   bool insert_into(table_id_t table_id, record_id_t record_id);
   bool delete_from(table_id_t table_id, record_id_t record_id);

   bool update_string(table_id_t table_id,
                      record_id_t record_id,
                      field_id_t field_id,
                      const std::string &value);
   bool update_int32(table_id_t table_id,
                     record_id_t record_id,
                     field_id_t field_id,
                     int32_t value);
   bool update_int64(table_id_t table_id,
                     record_id_t record_id,
                     field_id_t field_id,
                     int64_t value);
   bool update_reference(table_id_t table_id,
                         record_id_t record_id,
                         field_id_t field_id,
                         record_id_t value);
   bool update_boolean(table_id_t table_id,
                       record_id_t record_id,
                       field_id_t field_id,
                       bool value);
 };
}

#endif
