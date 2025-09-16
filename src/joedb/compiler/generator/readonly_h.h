#ifndef joedb_generator_readonly_h_declared
#define joedb_generator_readonly_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 /// @ingroup compiler
 class readonly_h: public Generator
 {
  public:
   readonly_h(const Compiler_Options &options):
    Generator(".", "readonly.h", options)
   {
   }

   void write(std::ostream &out) override
   {
    out << R"RRR(#include "Readonly_Database.h"
#include "Readonly_Client.h"
#include "Types.h"
//#warning readonly header is deprecated
)RRR";
    out.flush();
   }
 };
}

#endif
