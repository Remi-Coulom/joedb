#ifndef joedb_generator_struct_h_declared
#define joedb_generator_struct_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 class struct_h: public Generator
 {
  private:
   const std::pair<Table_Id, std::string> &table;

  public:
   struct_h
   (
    const Compiler_Options &options,
    const std::pair<Table_Id, std::string> &table
   );

   void generate() override;
 };
}

#endif
