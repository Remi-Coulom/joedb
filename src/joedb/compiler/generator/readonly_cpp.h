#ifndef joedb_generator_readonly_cpp_declared
#define joedb_generator_readonly_cpp_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// \ingroup compiler
 class readonly_cpp: public Generator
 {
  public:
   readonly_cpp(const Compiler_Options &options):
    Generator(".", "readonly.cpp", options)
   {
   }

   void generate() override
   {
    out << "#include \"Database.cpp\"\n";
   }
 };
}

#endif
