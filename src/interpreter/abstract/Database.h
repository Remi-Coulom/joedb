#ifndef crazydb_Database_declared
#define crazydb_Database_declared

#include <map>

#include "Table.h"

namespace crazydb
{
 class Database
 {
  private:
   std::map<std::string, Table> tables;

  public:
   const std::map<std::string, Table> &get_tables() const {return tables;}

   bool create_table(const std::string &name)
   {
    return tables.insert(std::make_pair(name, Table())).second;
   }

   bool drop_table(const std::string &name)
   {
    return tables.erase(name) > 0;
   }

   bool alter_table_add(const std::string &table_name,
                        const std::string &field_name,
                        const Type &field_type)
   {
    auto it = tables.find(table_name);

    if (it == tables.end())
     return false;
    else
     return it->second.add_field(field_name, field_type);
   }

   bool alter_table_drop(const std::string &table_name,
                         const std::string &field_name)
   {
    auto it = tables.find(table_name);

    if (it == tables.end())
     return false;
    else
     return it->second.drop_field(field_name);
   }
 };
}

#endif
