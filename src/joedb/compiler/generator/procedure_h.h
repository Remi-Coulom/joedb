#ifndef joedb_generator_procedure_h_declared
#define joedb_generator_procedure_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 class procedure_h: public Generator
 {
  private:
   const Compiler_Options &options;

  public:
   procedure_h
   (
    const Compiler_Options &options,
    const Compiler_Options &parent_options
   );

   void generate() override;
 };
}

#endif
