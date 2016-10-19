#ifndef joedb_Compiler_Options_io_declared
#define joedb_Compiler_Options_io_declared

#include <iosfwd>

namespace joedb
{
 class Compiler_Options;

 bool parse_compiler_options(std::istream &in,
                             Compiler_Options &compiler_options);
}

#endif
