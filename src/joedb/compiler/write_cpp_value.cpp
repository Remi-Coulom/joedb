#include "joedb/compiler/write_cpp_value.h"
#include "joedb/ui/write_value.h"
#include "joedb/ui/type_io.h"
#include "joedb/Readable.h"

#include <cmath>
#include <locale>
#include <sstream>
#include <type_traits>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 template<typename T> static void write_cpp_float(std::ostream &out, T value)
 ////////////////////////////////////////////////////////////////////////////
 {
  const char *type = std::is_same_v<T, float> ? "float" : "double";
  if (std::isnan(value))
   out << "std::numeric_limits<" << type << ">::quiet_NaN()";
  else if (std::isinf(value))
  {
   if (std::signbit(value))
    out << '-';
   out << "std::numeric_limits<" << type << ">::infinity()";
  }
  else
  {
   std::ostringstream literal;
   literal.imbue(std::locale::classic());
   literal << std::hexfloat << value;
   out << literal.str();
   if constexpr (std::is_same_v<T, float>)
    out << 'f';
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void write_cpp_value
 ////////////////////////////////////////////////////////////////////////////
 (
  std::ostream &out,
  const Readable &readable,
  Table_Id table_id,
  Record_Id record_id,
  Field_Id field_id
 )
 {
  const Type &type = readable.get_field_type(table_id, field_id);
  switch (type.get_type_id())
  {
   case Type::Type_Id::string:
   {
    const auto &value = readable.get_string(table_id, record_id, field_id);
    out << "std::string(";
    write_string(out, value);
    out << ", " << value.size() << ')';
   }
   break;

   case Type::Type_Id::blob:
   {
    const auto value = readable.get_blob(table_id, record_id, field_id);
    out << "joedb::Blob(" << value.get_position() << ", " << value.get_size() << ')';
   }
   break;

   case Type::Type_Id::float32:
    write_cpp_float(out, readable.get_float32(table_id, record_id, field_id));
   break;

   case Type::Type_Id::float64:
    write_cpp_float(out, readable.get_float64(table_id, record_id, field_id));
   break;

   case Type::Type_Id::reference:
    out << "id_of_" << readable.get_table_name(type.get_table_id()) << '(';
    write_reference(out, readable.get_reference(table_id, record_id, field_id));
    out << ')';
   break;

   default:
    write_value(out, readable, table_id, record_id, field_id);
   break;
  }
 }

}
