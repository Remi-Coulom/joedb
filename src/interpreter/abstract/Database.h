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
   bool create_table(const std::string &name)
   {
    return schema.create_table(name);
   }

   bool drop_table(const std::string &name)
   {
    return schema.drop_table(name);
   }

   bool alter_table_add(const std::string &table_name,
                        const std::string &field_name,
                        Type field_type)
   {
    return schema.alter_table_add(table_name, field_name, field_type);
   }

   bool alter_table_drop(const std::string &table_name,
                         const std::string &field_name)
   {
    return schema.alter_table_drop(table_name, field_name);
   }

   const Schema &get_schema() const {return schema;}
 };
}

#endif
