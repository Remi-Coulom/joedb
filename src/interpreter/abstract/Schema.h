#ifndef crazydb_Schema_declared
#define crazydb_Schema_declared

#include <map>

#include "Table.h"

namespace crazydb
{
 class Schema
 {
  private:
   std::map<std::string, Table> tables;

  public:
   bool create_table(const char *name);
   bool drop_table(const char *name);
   bool alter_table_add(const char *table_name,
                        const char *field_name,
                        Type field_type);
   bool alter_table_drop(const char *table_name,
                         const char *field_name);

   const Table *get_table(const char *table_name) const;
 };
}

#endif
