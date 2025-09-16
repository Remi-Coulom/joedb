#ifndef joedb_generator_File_Database_h_declared
#define joedb_generator_File_Database_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// @ingroup compiler
 class File_Database_h: public Generator
 {
  public:
   File_Database_h(const Compiler_Options &options);
   void write(std::ostream &out) override;
 };
}

#endif
