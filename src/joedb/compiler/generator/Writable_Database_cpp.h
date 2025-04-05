#ifndef joedb_generator_Writable_Database_cpp_declared
#define joedb_generator_Writable_Database_cpp_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// @ingroup compiler
 class Writable_Database_cpp: public Generator
 {
  public:
   Writable_Database_cpp(const Compiler_Options &options);
   void generate() override;
 };
}

#endif
