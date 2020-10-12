#ifndef joedb_Compiler_Options_io_declared
#define joedb_Compiler_Options_io_declared

#include <iosfwd>

namespace joedb
{
 class Compiler_Options;

 // true = OK, false = error
 bool parse_compiler_options
 (
  std::istream &in,
  std::ostream &out, // for errors and warnings
  Compiler_Options &compiler_options
 );
}

#endif
