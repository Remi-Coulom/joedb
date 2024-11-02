#include "joedb/compiler/generator/Database_h.h"

namespace joedb::generator
{
 Database_h::Database_h(const Compiler_Options &options):
  Generator(".", "Database.h", options)
 {
 }

 void Database_h::generate()
 {
  out << "// Hello\n";
 }
}
