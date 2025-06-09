#ifndef joedb_generator_Procedure_h_declared
#define joedb_generator_Procedure_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 class Procedure_h: public Generator
 {
  private:
   const Compiler_Options &parent_options;

  public:
   Procedure_h
   (
    const Compiler_Options &options,
    const Compiler_Options &parent_options
   );

   void generate() override;
 };
}

#endif
