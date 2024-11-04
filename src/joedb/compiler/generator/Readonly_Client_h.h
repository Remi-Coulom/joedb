#ifndef joedb_generator_Readonly_Client_h_declared
#define joedb_generator_Readonly_Client_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 class Readonly_Client_h: public Generator
 {
  public:
   Readonly_Client_h(const Compiler_Options &options);
   void generate() override;
 };
}

#endif