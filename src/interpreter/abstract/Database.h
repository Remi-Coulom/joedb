#ifndef crazydb_Database_declared
#define crazydb_Database_declared

#include "Schema.h"

namespace crazydb
{
 class Database
 {
  private:
   Schema schema;

  public:
   bool create_table(const char *name);
   bool drop_table(const char *name);
   bool alter_table_add(const char *table_name,
                        const char *field_name,
                        Type field_type);
   bool alter_table_drop(const char *table_name,
                         const char *field_name);

   const Schema &get_schema() const {return schema;}
 };
}

#endif
