#ifndef joedb_string_io_declared
#define joedb_string_io_declared

#include <iosfwd>
#include <cstdint>

namespace joedb
{
 std::string read_string(std::istream &in); 
 void write_string(std::ostream &out, const std::string &s);

 char get_hex_char_from_digit(uint8_t n);
 uint8_t get_hex_digit_from_char(char c);
 void write_hexa_character(std::ostream &out, uint8_t c);
}

#endif
