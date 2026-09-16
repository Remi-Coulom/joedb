#ifndef joedb_generator_Database_h_declared
#define joedb_generator_Database_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// @ingroup compiler
 class Database_h: public Generator
 {
  private:
   void write_index_parameters(std::ostream &out, const Compiler_Options::Index &index);

  public:
   Database_h(const Compiler_Options &options);
   void write(std::ostream &out) override;
 };
}

#endif
