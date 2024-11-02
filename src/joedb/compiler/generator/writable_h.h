#ifndef joedb_generator_writable_h_declared
#define joedb_generator_writable_h_declared

#include "joedb/compiler/generator/Generator.h"

namespace joedb::generator
{
 class writable_h: public Generator
 {
  public:
   writable_h(const Compiler_Options &options):
    Generator(".", "writable.h", options)
   {
   }

   void generate() override
   {
    out << R"RRR(#include "readonly.h"
#include "File_Database.h"
#include "Client.h"
#include "Local_Client.h"
)RRR";
   }
 };
}

#endif
