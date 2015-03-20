#ifndef joedb_utf8_declared
#define joedb_utf8_declared

#include <iosfwd>

namespace joedb
{
 // Parse and write utf8 strings with C++ syntax
 std::string read_utf8_string(std::istream &in); 
 void write_utf8_string(std::ostream &out,
                        const std::string &s,
                        bool ascii_only);
}

#endif
