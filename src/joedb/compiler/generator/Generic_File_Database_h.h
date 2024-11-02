#ifndef joedb_generator_Generic_File_Database_h_declared
#define joedb_generator_Generic_File_Database_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 class Generic_File_Database_h: public Generator
 {
  public:
   Generic_File_Database_h(const Compiler_Options &options);
   void generate() override;
 };
}

#endif
