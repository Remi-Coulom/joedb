#ifndef crazydb_TableFields_declared
#define crazydb_TableFields_declared

#include <string>
#include <map>

#include "Type.h"

namespace crazydb
{
 class TableFields
 {
  private:
   std::map<std::string, Type> fields;

  public:
   const std::map<std::string, Type> &get_fields() const {return fields;}

   bool add_field(const std::string &name, const Type &type)
   {
    return fields.insert(std::make_pair(name, type)).second;
   }

   bool drop_field(const std::string &name)
   {
    return fields.erase(name) > 0;
   }
 };
}

#endif
