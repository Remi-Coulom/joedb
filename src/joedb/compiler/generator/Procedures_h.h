#ifndef joedb_generator_Procedures_h_declared
#define joedb_generator_Procedures_h_declared

#include "joedb/compiler/generator/Generator.h"
#include "joedb/compiler/generator/Procedure.h"

#include <vector>

namespace joedb::generator
{
 class Procedures_h: public Generator
 {
  private:
   const std::vector<Procedure> &procedures;

  public:
   Procedures_h
   (
    const Compiler_Options &options,
    const std::vector<Procedure> &procedures
   );

   void write(std::ostream &out) override;
 };
}

#endif
