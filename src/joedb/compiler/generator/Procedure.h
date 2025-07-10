#ifndef joedb_generator_Procedure_declared
#define joedb_generator_Procedure_declared

#include <string>

namespace joedb::generator
{
 struct Procedure
 {
  std::string name;
  std::string schema;
  std::string include;
  enum {read, write} type;

  bool operator<(const Procedure &procedure) const
  {
   if (schema < procedure.schema)
    return true;
   if (schema > procedure.schema)
    return false;

   if (name < procedure.name)
    return true;
   if (name > procedure.name)
    return false;

   return false;
  }
 };
}

#endif
