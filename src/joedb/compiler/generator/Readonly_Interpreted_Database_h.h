#ifndef joedb_generator_Readonly_Interpreted_Database_h_declared
#define joedb_generator_Readonly_Interpreted_Database_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// \ingroup compiler
 class Readonly_Interpreted_Database_h: public Generator
 {
  public:
   Readonly_Interpreted_Database_h(const Compiler_Options &options);
   void generate() override;
 };
}

#endif
