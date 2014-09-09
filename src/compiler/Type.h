#ifndef crazydb_Type_declared
#define crazydb_Type_declared

#include <iosfwd>

namespace crazydb
{
 class Type
 {
  public:
   virtual void generate_private(std::ostream &out) {}
   virtual void generate_public(std::ostream &out) {}
 };
}

#endif
