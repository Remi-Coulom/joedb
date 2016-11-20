#ifndef joedb_c_wrapper_declared
#define joedb_c_wrapper_declared

#include <iosfwd>

namespace joedb
{
 class Compiler_Options;

 void generate_c_wrapper(std::ostream &header,
                         std::ostream &body,
                         const joedb::Compiler_Options &compiler_options);
}

#endif
