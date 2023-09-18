#ifndef joedb_index_types_declared
#define joedb_index_types_declared

#include <stdint.h>
#include <stdlib.h>
#include <type_traits>

namespace joedb
{
 enum class Table_Id: uint16_t {};
 enum class Field_Id: uint16_t {};
 enum class Record_Id: size_t {};

 inline std::underlying_type<Table_Id>::type to_underlying(Table_Id id)
 {
  return std::underlying_type<Table_Id>::type(id);
 }
 inline std::underlying_type<Field_Id>::type to_underlying(Field_Id id)
 {
  return std::underlying_type<Field_Id>::type(id);
 }
 inline std::underlying_type<Record_Id>::type to_underlying(Record_Id id)
 {
  return std::underlying_type<Record_Id>::type(id);
 }

 inline Record_Id operator+(Record_Id id, size_t size)
 {
  return Record_Id(to_underlying(id) + size);
 }

 inline Table_Id &operator++(Table_Id &id)
 {
  return id = Table_Id(to_underlying(id) + 1);
 }
 inline Field_Id &operator++(Field_Id &id)
 {
  return id = Field_Id(to_underlying(id) + 1);
 }
 inline Record_Id &operator++(Record_Id &id)
 {
  return id = id + 1;
 }
}

#endif
