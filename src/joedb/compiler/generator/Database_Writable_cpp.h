#ifndef joedb_generator_Database_Writable_cpp_declared
#define joedb_generator_Database_Writable_cpp_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// @ingroup compiler
 class Database_Writable_cpp: public Generator
 {
  public:
   Database_Writable_cpp(const Compiler_Options &options);
   void write(std::ostream &out) override;
 };
}

#endif
