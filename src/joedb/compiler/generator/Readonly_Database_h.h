#ifndef joedb_generator_Readonly_Database_h_declared
#define joedb_generator_Readonly_Database_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::compiler::generator
{
 class Readonly_Database_h: public Generator
 {
  public:
   Readonly_Database_h(const Compiler_Options &options);
   void generate() override;
 };
}

#endif
