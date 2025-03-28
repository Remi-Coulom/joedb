#ifndef joedb_generator_Local_Client_h_declared
#define joedb_generator_Local_Client_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// \ingroup compiler
 class Local_Client_h: public Generator
 {
  public:
   Local_Client_h(const Compiler_Options &options);
   void generate() override;
 };
}

#endif
