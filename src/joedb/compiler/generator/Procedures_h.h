#ifndef joedb_generator_Procedures_h_declared
#define joedb_generator_Procedures_h_declared

#include "joedb/compiler/generator/Generator.h"
#include "joedb/compiler/generator/Procedure.h"

#include <set>

namespace joedb::generator
{
 class Procedures_h: public Generator
 {
  private:
   const std::set<Procedure> &procedures;

  public:
   Procedures_h
   (
    const Compiler_Options &options,
    const std::set<Procedure> &procedures
   );

   void generate() override;
 };
}

#endif
