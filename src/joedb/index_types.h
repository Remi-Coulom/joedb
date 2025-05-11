#ifndef joedb_index_types_declared
#define joedb_index_types_declared

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <type_traits>

namespace joedb
{
 /// @ingroup joedb
 enum class Table_Id: uint16_t {};

 /// @ingroup joedb
 enum class Field_Id: uint16_t {};

 /// @ingroup joedb
 using index_t = ptrdiff_t;

 /// @ingroup joedb
 class Record_Id
 {
  private:
   index_t id;

  public:
   constexpr explicit Record_Id(): id(-1) {}
   constexpr explicit Record_Id(index_t id): id(id) {}
   constexpr explicit operator index_t() const {return id;}
   constexpr explicit operator size_t() const {return size_t(id);}
   constexpr Record_Id operator+(size_t n) const {return Record_Id(id + n);}
   constexpr Record_Id operator-(size_t n) const {return Record_Id(id - n);}
   constexpr Record_Id operator++() {return Record_Id(++id);}
   constexpr Record_Id operator--() {return Record_Id(--id);}
   constexpr bool operator==(Record_Id r) const {return id == r.id;}
   constexpr bool operator!=(Record_Id r) const {return id != r.id;}
   constexpr bool operator<=(Record_Id r) const {return id <= r.id;}
   constexpr bool operator>=(Record_Id r) const {return id >= r.id;}
   constexpr bool operator<(Record_Id r) const {return id < r.id;}
   constexpr bool operator>(Record_Id r) const {return id > r.id;}
   constexpr bool is_null() const {return id < 0;}
   constexpr bool is_not_null() const {return id >= 0;}

   static const Record_Id null;
 };

 inline constexpr Record_Id Record_Id::null{};

 template<typename T> struct underlying_type
 {
  using type = typename std::underlying_type<T>::type;
 };

 template<> struct underlying_type<Record_Id>
 {
  using type = index_t;
 };

 constexpr index_t to_underlying(Record_Id id) {return index_t(id);}

 constexpr underlying_type<Table_Id>::type to_underlying
 (
  Table_Id id
 )
 {
  return underlying_type<Table_Id>::type(id);
 }

 constexpr underlying_type<Field_Id>::type to_underlying
 (
  Field_Id id
 )
 {
  return underlying_type<Field_Id>::type(id);
 }

 inline Table_Id &operator++(Table_Id &id)
 {
  return id = Table_Id(to_underlying(id) + 1);
 }
 inline Field_Id &operator++(Field_Id &id)
 {
  return id = Field_Id(to_underlying(id) + 1);
 }
}

#endif
