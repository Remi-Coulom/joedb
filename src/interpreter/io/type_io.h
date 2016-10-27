#ifndef joedb_type_io_declared
#define joedb_type_io_declared

#include <iostream>
#include <cstdint>
#include "index_types.h"

namespace joedb
{
 std::string read_string(std::istream &in); 
 void write_string(std::ostream &out, const std::string &s);

 char get_hex_char_from_digit(uint8_t n);
 uint8_t get_hex_digit_from_char(char c);
 void write_hexa_character(std::ostream &out, uint8_t c);
 void write_octal_character(std::ostream &out, uint8_t c);

 #define PRIMITIVE_IO(type, type_id)\
 inline type read_##type_id(std::istream &in)\
 {type value = type(); in >> value; return value;}\
 inline void write_##type_id(std::ostream &out, type value)\
 {out << value;}
 PRIMITIVE_IO(int32_t, int32)
 PRIMITIVE_IO(int64_t, int64)
 PRIMITIVE_IO(bool, boolean)
 PRIMITIVE_IO(record_id_t, reference)
 PRIMITIVE_IO(float, float32)
 PRIMITIVE_IO(double, float64)
 PRIMITIVE_IO(int8_t, int8)
 #undef PRIMITIVE_IO
}

#endif
