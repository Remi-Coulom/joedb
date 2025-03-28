#ifndef joedb_generator_Buffered_File_Database_cpp_declared
#define joedb_generator_Buffered_File_Database_cpp_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// \ingroup compiler
 class Buffered_File_Database_cpp: public Generator
 {
  public:
   Buffered_File_Database_cpp(const Compiler_Options &options);
   void generate() override;
 };
}

#endif
