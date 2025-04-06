#ifndef joedb_generator_introspection_h_declared
#define joedb_generator_introspection_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// @ingroup compiler
 class introspection_h: public Generator
 {
  private:
   const std::pair<Table_Id, std::string> &table;

  public:
   introspection_h
   (
    const Compiler_Options &options,
    const std::pair<Table_Id, std::string> &table
   );

   void generate() override;
 };
}

#endif
