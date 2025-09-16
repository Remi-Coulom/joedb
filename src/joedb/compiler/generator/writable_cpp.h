#ifndef joedb_generator_writable_cpp_declared
#define joedb_generator_writable_cpp_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// @ingroup compiler
 class writable_cpp: public Generator
 {
  public:
   writable_cpp(const Compiler_Options &options):
    Generator(".", "writable.cpp", options)
   {
   }

   void write(std::ostream &out) override
   {
    out << "#include \"readonly.cpp\"\n";
    out << "#include \"Writable_Database.cpp\"\n";
    out.flush();
   }
 };
}

#endif
