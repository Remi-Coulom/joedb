#ifndef crazydb_Table_declared
#define crazydb_Table_declared

#include <string>
#include <map>

#include "Type.h"

namespace crazydb
{
 class Table
 {
  private:
   std::map<std::string, Type> fields;

  public:
   const std::map<std::string, Type> &get_fields() const {return fields;}

   bool add_field(const char *name, const Type &type)
   {
    std::string name_string(name);

    if (fields.count(name_string))
     return false;
    else
    {
     fields.insert(std::make_pair(name_string, type));
     return true;
    }
   }
 };
}

#endif
