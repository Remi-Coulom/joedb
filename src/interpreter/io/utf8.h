#ifndef joedb_utf8_declared
#define joedb_utf8_declared

#include <iosfwd>
#include <cstdint>

namespace joedb
{
 std::string read_utf8_string(std::istream &in); 
 void write_utf8_string(std::ostream &out, const std::string &s);

 char get_hex_char_from_digit(uint8_t n);
 uint8_t get_hex_digit_from_char(char c);
 void write_hexa_character(std::ostream &out, uint8_t c);
}

#endif
