#ifndef crazydb_Database_declared
#define crazydb_Database_declared

#include "Table.h"

namespace crazydb
{
 class Database
 {
  private:
   table_id_t current_table_id;

   std::unordered_map<table_id_t, Table> tables;

  public:
   Database(): current_table_id(0) {}

   const std::unordered_map<table_id_t, Table> &get_tables() const
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

   table_id_t find_table(const std::string &name)
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

   record_id_t insert_into(table_id_t table_id)
   {
    auto it = tables.find(table_id);
    if (it == tables.end())
     return 0;
    return it->second.insert_record();
   }

   bool update(table_id_t table_id,
               record_id_t record_id,
               field_id_t field_id,
               const Value &value)
   {
    auto it = tables.find(table_id);
    if (it == tables.end())
     return 0;
    return it->second.update(record_id, field_id, value);
   }

   bool drop_field(table_id_t table_id, field_id_t field_id)
   {
    auto it = tables.find(table_id);
    if (it == tables.end())
     return 0;
    return it->second.drop_field(field_id);
   }
 };
}

#endif
