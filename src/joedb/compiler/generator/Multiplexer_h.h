#ifndef joedb_generator_Multiplexer_h_declared
#define joedb_generator_Multiplexer_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// @ingroup compiler
 class Multiplexer_h: public Generator
 {
  public:
   Multiplexer_h(const Compiler_Options &options);
   void generate() override;
 };
}

#endif
