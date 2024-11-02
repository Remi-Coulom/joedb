#ifndef joedb_generator_Database_cpp_declared
#define joedb_generator_Database_cpp_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 class Database_cpp: public Generator
 {
  public:
   Database_cpp(const Compiler_Options &options);
   void generate() override;
 };
}

#endif
