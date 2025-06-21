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
 };
}

#endif
