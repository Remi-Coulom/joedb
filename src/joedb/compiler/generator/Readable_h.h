#ifndef joedb_generator_Readable_h_declared
#define joedb_generator_Readable_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// @ingroup compiler
 class Readable_h: public Generator
 {
  public:
   Readable_h(const Compiler_Options &options);
   void write(std::ostream &out) override;
 };
}

#endif
