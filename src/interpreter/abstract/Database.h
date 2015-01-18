#ifndef joedb_Database_declared
#define joedb_Database_declared

#include "Table.h"

namespace joedb
{
 class Database
 {
  private:
   table_id_t current_table_id;

   std::map<table_id_t, Table> tables;

  public:
   Database(): current_table_id(0) {}

   const std::map<table_id_t, Table> &get_tables() const
   {
    return tables;
   }

   table_id_t create_table(const std::string &name)
   {
    tables.insert(std::make_pair(++current_table_id, Table(name)));
    return current_table_id;
   }

   bool drop_table(table_id_t table_id)
   {
    return tables.erase(table_id) > 0;
   }

   table_id_t find_table(const std::string &name) const
   {
    for (const auto &table: tables)
     if (table.second.get_name() == name)
      return table.first;
    return 0;
   }

   field_id_t add_field(table_id_t table_id,
                        const std::string &name,
                        Type type)
   {
    auto it = tables.find(table_id);
    if (it == tables.end())
     return 0;
    return it->second.add_field(name, type);
   }

   field_id_t find_field(table_id_t table_id,
                         const std::string &name) const
   {
    auto it = tables.find(table_id);
    if (it == tables.end())
     return 0;
    return it->second.find_field(name);
   }

   Type::type_id_t get_field_type(table_id_t table_id,
                                  field_id_t field_id) const
   {
    auto table_it = tables.find(table_id);
    if (table_it == tables.end())
     return Type::null_id;
    auto &fields = table_it->second.get_fields();
    auto field_it = fields.find(field_id);
    if (field_it == fields.end())
     return Type::null_id;
    return field_it->second.type.get_type_id();
   }

   bool drop_field(table_id_t table_id, field_id_t field_id)
   {
    auto it = tables.find(table_id);
    if (it == tables.end())
     return 0;
    return it->second.drop_field(field_id);
   }

   record_id_t insert_into(table_id_t table_id)
   {
    auto it = tables.find(table_id);
    if (it == tables.end())
     return 0;
    return it->second.insert_record();
   }

   record_id_t insert_into(table_id_t table_id, record_id_t record_id)
   {
    auto it = tables.find(table_id);
    if (it == tables.end())
     return 0;
    return it->second.insert_record(record_id);
   }

   bool update(table_id_t table_id,
               record_id_t record_id,
               field_id_t field_id,
               const Value &value)
   {
    auto it = tables.find(table_id);
    if (it == tables.end())
     return false;
    return it->second.update(record_id, field_id, value);
   }

   bool delete_record(table_id_t table_id, record_id_t record_id)
   {
    auto it = tables.find(table_id);
    if (it == tables.end())
     return false;
    return it->second.delete_record(record_id);
   }
 };
}

#endif
