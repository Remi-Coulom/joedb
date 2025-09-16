#ifndef joedb_generator_Writable_Database_h_declared
#define joedb_generator_Writable_Database_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// @ingroup compiler
 class Writable_Database_h: public Generator
 {
  public:
   Writable_Database_h(const Compiler_Options &options);
   void write(std::ostream &out) override;
 };
}

#endif
