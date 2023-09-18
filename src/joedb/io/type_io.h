#ifndef joedb_type_io_declared
#define joedb_type_io_declared

#include "joedb/index_types.h"
#include "joedb/Blob.h"

#include <iostream>
#include <cstdint>

namespace joedb
{
 std::string read_string(std::istream &in);
 void write_string(std::ostream &out, const std::string &s, bool json = false);
 void write_sql_string(std::ostream &out, const std::string &s);

 size_t utf8_display_size(const std::string &s);
 uint32_t read_utf8_char(size_t &i, const std::string &s);
 void write_justified
 (
  std::ostream &out,
  const std::string &s,
  size_t width,
  bool flush_left = true
 );

 char get_hex_char_from_digit(uint8_t n);
 uint8_t get_hex_digit_from_char(char c);
 void write_hexa_character(std::ostream &out, uint8_t c);
 void write_octal_character(std::ostream &out, uint8_t c);

 int8_t read_int8(std::istream &in);
 void write_int8(std::ostream &out, int8_t value);

 void write_blob(std::ostream &out, const Blob blob);
 Blob read_blob(std::istream &in);

 #define PRIMITIVE_IO(type, type_id)\
 type read_##type_id(std::istream &in);\
 void write_##type_id(std::ostream &out, type value);

 PRIMITIVE_IO(int32_t, int32)
 PRIMITIVE_IO(int64_t, int64)
 PRIMITIVE_IO(bool, boolean)
 PRIMITIVE_IO(Record_Id, reference)
 PRIMITIVE_IO(float, float32)
 PRIMITIVE_IO(double, float64)
 PRIMITIVE_IO(int16_t, int16)

 #undef PRIMITIVE_IO

 inline std::ostream &operator<<(std::ostream &out, Table_Id table_id)
 {
  return out << to_underlying(table_id);
 }
 inline std::ostream &operator<<(std::ostream &out, Field_Id field_id)
 {
  return out << to_underlying(field_id);
 }
 inline std::ostream &operator<<(std::ostream &out, Record_Id record_id)
 {
  return out << to_underlying(record_id);
 }
}

#endif
