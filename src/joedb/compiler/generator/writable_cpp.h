#ifndef joedb_generator_writable_cpp_declared
#define joedb_generator_writable_cpp_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 class writable_cpp: public Generator
 {
  public:
   writable_cpp(const Compiler_Options &options):
    Generator(".", "writable.cpp", options)
   {
   }

   void generate() override
   {
    out << "#include \"readonly.cpp\"\n";
    out << "#include \"Generic_File_Database.cpp\"\n";
   }
 };
}

#endif
