#ifndef joedb_generator_Client_h_declared
#define joedb_generator_Client_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// @ingroup compiler
 class Client_h: public Generator
 {
  public:
   Client_h(const Compiler_Options &options);
   void generate() override;
 };
}

#endif
