#ifndef crazydb_Table_type_declared
#define crazydb_Table_type_declared

#include "Type.h"

namespace crazydb
{
 class Table_type: public Type
 {
  private:
   const Table &table;

  public:
   Table_type(const Table &table): table(table) {}

   void generate_private(std::ostream &out) {}
   void generate_public(std::ostream &out) {}
 };
}

#endif
