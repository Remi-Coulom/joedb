#ifndef joedb_generator_Procedures_cpp_declared
#define joedb_generator_Procedures_cpp_declared

#include "joedb/compiler/generator/Generator.h"
#include "joedb/compiler/generator/Procedure.h"

#include <vector>

namespace joedb::generator
{
 class Procedures_cpp: public Generator
 {
  private:
   const std::vector<Procedure> &procedures;

  public:
   Procedures_cpp
   (
    const Compiler_Options &options,
    const std::vector<Procedure> &procedures
   );

   void generate() override;
 };
}

#endif
