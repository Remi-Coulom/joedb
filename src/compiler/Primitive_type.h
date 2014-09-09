#ifndef crazydb_Primitive_type_declared
#define crazydb_Primitive_type_declared

#include "Type.h"

namespace crazydb
{
 class Primitive_type: public Type
 {
  private:
   std::string cpp_type;

  public:
   Primitive_type(const std::string &cpp_type): cpp_type(cpp_type) {}

   void generate_private(std::ostream &out) {}
   void generate_public(std::ostream &out) {}
 };
}

#endif
