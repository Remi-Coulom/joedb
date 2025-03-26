#ifndef joedb_generator_Readable_h_declared
#define joedb_generator_Readable_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::compiler::generator
{
 class Readable_h: public Generator
 {
  public:
   Readable_h(const Compiler_Options &options);
   void generate() override;
 };
}

#endif
