#ifndef joedb_generator_RPC_Client_h_declared
#define joedb_generator_RPC_Client_h_declared

#include "joedb/compiler/generator/Generator.h"
#include "joedb/compiler/generator/Procedure.h"

#include <vector>

namespace joedb::generator
{
 class RPC_Client_h: public Generator
 {
  private:
   const std::vector<Procedure> &procedures;

  public:
   RPC_Client_h
   (
    const Compiler_Options &options,
    const std::vector<Procedure> &procedures
   );

   void generate() override;
 };
}

#endif
