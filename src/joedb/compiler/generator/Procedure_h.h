#ifndef joedb_generator_Procedure_h_declared
#define joedb_generator_Procedure_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 class Procedure_h: public Generator
 {
  public:
   Procedure_h(const Compiler_Options &options);

   void generate() override;
 };
}

#endif
