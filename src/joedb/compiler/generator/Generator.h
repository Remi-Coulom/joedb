#ifndef joedb_generator_Generator_declared
#define joedb_generator_Generator_declared

#include "joedb/compiler/Compiler_Options.h"

#include <fstream>

namespace joedb::generator
{
 class Generator
 {
  protected:
   const Compiler_Options &options;
   std::ofstream out;

   void write_initial_comment();

  public:
   Generator
   (
    const char *dir_name,
    const char *file_name,
    const Compiler_Options &options
   );

   virtual void generate();
   virtual ~Generator();
 };
}

#endif
