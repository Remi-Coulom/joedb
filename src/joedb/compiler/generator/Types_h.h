#ifndef joedb_generator_Types_h_declared
#define joedb_generator_Types_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// @ingroup compiler
 class Types_h: public Generator
 {
  public:
   Types_h(const Compiler_Options &options);
   void write(std::ostream &out) override;
 };
}

#endif
