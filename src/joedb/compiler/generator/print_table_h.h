#ifndef joedb_generator_print_table_h_declared
#define joedb_generator_print_table_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// @ingroup compiler
 class print_table_h: public Generator
 {
  public:
   print_table_h(const Compiler_Options &options);
   void write(std::ostream &out) override;
 };
}

#endif
