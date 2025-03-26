#ifndef joedb_generator_Buffered_File_Database_h_declared
#define joedb_generator_Buffered_File_Database_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::compiler::generator
{
 class Buffered_File_Database_h: public Generator
 {
  public:
   Buffered_File_Database_h(const Compiler_Options &options);
   void generate() override;
 };
}

#endif
