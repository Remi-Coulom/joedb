#ifndef joedb_generator_Database_Writable_h_declared
#define joedb_generator_Database_Writable_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// @ingroup compiler
 class Database_Writable_h: public Generator
 {
  public:
   Database_Writable_h(const Compiler_Options &options);
   void generate() override;
 };
}

#endif
