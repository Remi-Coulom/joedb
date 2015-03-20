#ifndef joedb_utf8_declared
#define joedb_utf8_declared

#include <iosfwd>
#include <cstdint>

namespace joedb
{
 std::string read_utf8_string(std::istream &in); 

 void write_hexa_character(std::ostream &out, uint8_t c);
 void write_utf8_string(std::ostream &out,
                        const std::string &s,
                        bool ascii_only);
}

#endif
