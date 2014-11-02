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

   Table &create_table(const std::string &name)
   {
    return tables.insert(std::make_pair(name, Table())).first->second;
   }

   bool drop_table(const std::string &name)
   {
    return tables.erase(name) > 0;
   }

   Table *get_table(const std::string &name)
   {
    auto it = tables.find(name);

    if (it == tables.end())
     return 0;
    else
     return &it->second;
   }
 };
}

#endif
