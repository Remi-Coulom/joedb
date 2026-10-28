#include "joedb/compiler/write_cpp_value.h"
#include "joedb/interpreted/Database.h"

#include "gtest/gtest.h"

#include <sstream>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <type_traits>

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler_Test, write_cpp_value)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Database db;
 db.create_table("value");
 const auto table = db.find_table("value");
 db.add_field(table, "single", joedb::Type::float32());
 db.add_field(table, "double_precision", joedb::Type::float64());
 db.add_field(table, "blob", joedb::Type::blob());
 const joedb::Record_Id record{0};
 db.insert_into(table, record);

 const auto check_float = [&]<typename T>(T)
 {
  const auto field = db.find_field(table, std::is_same_v<T, float> ? "single" : "double_precision");
  const auto write = [&](T value)
  {
   if constexpr (std::is_same_v<T, float>)
    db.update_float32(table, record, field, value);
   else
    db.update_float64(table, record, field, value);
   std::ostringstream out;
   joedb::write_cpp_value(out, db, table, record, field);
   return out.str();
  };

  for (const T value: {T(0), -T(0), T(1.2345678901234567), std::numeric_limits<T>::min(),
                      std::numeric_limits<T>::max(), std::numeric_limits<T>::denorm_min()})
  {
   const std::string literal = write(value);
   char *end;
   const T parsed = T(std::strtod(literal.c_str(), &end));
   EXPECT_EQ(parsed, value) << literal;
   EXPECT_EQ(std::signbit(parsed), std::signbit(value));
   EXPECT_STREQ(end, (std::is_same_v<T, float> ? "f" : ""));
  }

  const std::string prefix = std::is_same_v<T, float> ? "std::numeric_limits<float>" : "std::numeric_limits<double>";
  EXPECT_EQ(write(std::numeric_limits<T>::infinity()), prefix + "::infinity()");
  EXPECT_EQ(write(-std::numeric_limits<T>::infinity()), "-" + prefix + "::infinity()");
  EXPECT_EQ(write(std::numeric_limits<T>::quiet_NaN()), prefix + "::quiet_NaN()");
 };
 check_float(float{});
 check_float(double{});

 const auto blob = db.find_field(table, "blob");
 db.update_blob(table, record, blob, joedb::Blob(123, 456));
 std::ostringstream out;
 joedb::write_cpp_value(out, db, table, record, blob);
 EXPECT_EQ(out.str(), "joedb::Blob(123, 456)");
}

