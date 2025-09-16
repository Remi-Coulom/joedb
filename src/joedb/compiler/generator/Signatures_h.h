#ifndef joedb_generator_Signatures_h_declared
#define joedb_generator_Signatures_h_declared

#include "joedb/compiler/generator/Generator.h"
#include "joedb/compiler/generator/Procedure.h"

#include <vector>

namespace joedb::generator
{
 class Signatures_h: public Generator
 {
  private:
   const std::vector<Procedure> &procedures;

  public:
   Signatures_h
   (
    const Compiler_Options &options,
    const std::vector<Procedure> &procedures
   );

   void write(std::ostream &out) override;
 };
}

#endif
