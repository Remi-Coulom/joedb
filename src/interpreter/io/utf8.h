#ifndef joedb_utf8_declared
#define joedb_utf8_declared

#include <iosfwd>
#include <cstdint>

namespace joedb
{
 std::string read_utf8_string(std::istream &in); 

 void write_hexa_character(std::ostream &out, uint8_t c);
 char get_hex_digit(uint8_t n);
 void write_utf8_string(std::ostream &out, const std::string &s);
}

#endif
