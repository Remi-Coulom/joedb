#ifndef crazydb_Table_declared
#define crazydb_Table_declared

#include <map>
#include <vector>

#include "Type.h"

namespace crazydb
{
 class Table
 {
  private:
   std::map<std::string, Type> fields;
   std::map<uint64_t, std::vector<void *> > records;

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

   ~Table()
   {
   }
 };
}

#endif
