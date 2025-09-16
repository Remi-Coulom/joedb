#ifndef joedb_generator_ids_h_declared
#define joedb_generator_ids_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// @ingroup compiler
 class ids_h: public Generator
 {
  private:
   const Compiler_Options *parent_options;

  public:
   ids_h
   (
    const Compiler_Options &options,
    const Compiler_Options *parent_options
   );
   void write(std::ostream &out) override;
 };
}

#endif
