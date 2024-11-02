#ifndef joedb_generator_readonly_cpp_declared
#define joedb_generator_readonly_cpp_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 class readonly_cpp: public Generator
 {
  public:
   readonly_cpp(const Compiler_Options &options);
   void generate() override;
 };
}

#endif
