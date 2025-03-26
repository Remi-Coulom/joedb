#ifndef joedb_generator_ids_h_declared
#define joedb_generator_ids_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::compiler::generator
{
 class ids_h: public Generator
 {
  public:
   ids_h(const Compiler_Options &options);
   void generate() override;
 };
}

#endif
