#include "joedb/ui/type_io.h"
#include "joedb/ui/write_value.h"
#include "joedb/interpreted/Database.h"
#include "joedb/ui/base64.h"
#include "joedb/error/Exception.h"
#include "external/wide_char_display_width.h"

#include "gtest/gtest.h"

#include <sstream>
#include <random>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <type_traits>

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, write_cpp_value)
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

struct
{
 std::string s1;
 std::string s2;
}
pairs[] =
{
 {"Hello"           , "\"Hello\""},
 {"Rémi"            , "\"Rémi\""},
 {"Hello, world!"   , "\"Hello, world!\""},
 {"\"\\\x1f"        , "\"\\\"\\\\\\037\""},
 {"これは日本語です", "\"これは日本語です\""},
 {"𩸽"              , "\"𩸽\""}, // 4-byte character
 {""                , "\"\""},
 {"4"               , "\"4\""}
};

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, wide_char_display_width)
/////////////////////////////////////////////////////////////////////////////
{
 EXPECT_EQ(1, wide_char_display_width('e'));
 EXPECT_EQ(1, wide_char_display_width(0x00e9));
 {
  size_t i = 0;
  std::string s = "日";
  uint32_t c = joedb::read_utf8_char(i, s);
  EXPECT_EQ(size_t(3), i);
  EXPECT_EQ(size_t(3), s.size());
  EXPECT_EQ(uint8_t(0xe6), uint8_t(s[0]));
  EXPECT_EQ(uint8_t(0x97), uint8_t(s[1]));
  EXPECT_EQ(uint8_t(0xa5), uint8_t(s[2]));
  EXPECT_EQ(uint32_t(0x65e5), c);
  EXPECT_EQ(int(2), wide_char_display_width(c));
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, read_utf8_char)
/////////////////////////////////////////////////////////////////////////////
{
 {
  size_t i = 1;
  std::string s = "Rémi";
  uint32_t c = joedb::read_utf8_char(i, s);
  EXPECT_EQ(size_t(3), i);
  EXPECT_EQ(uint32_t(0x00e9), c);
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, utf8_display_size)
/////////////////////////////////////////////////////////////////////////////
{
 EXPECT_EQ(size_t(0), joedb::utf8_display_size(""));
 EXPECT_EQ(size_t(4), joedb::utf8_display_size("Remi"));
 EXPECT_EQ(size_t(4), joedb::utf8_display_size("Rémi"));
 EXPECT_EQ(size_t(4), joedb::utf8_display_size("山下"));
 EXPECT_EQ(size_t(2), joedb::utf8_display_size("𩸽"));
 EXPECT_EQ(size_t(4), joedb::utf8_display_size("바둑"));
 EXPECT_EQ(size_t(4), joedb::utf8_display_size("圍棋"));
 EXPECT_EQ(size_t(4), joedb::utf8_display_size("围棋"));
 EXPECT_EQ(size_t(4), joedb::utf8_display_size("囲碁"));
}

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, write_hexa_character)
/////////////////////////////////////////////////////////////////////////////
{
 std::ostringstream out;
 joedb::write_hexa_character(out, 0x1a);
 EXPECT_EQ("\\x1a", out.str());
}

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, read_string)
/////////////////////////////////////////////////////////////////////////////
{
 for (auto p: pairs)
 {
  std::istringstream iss(p.s2);
  std::string s = joedb::read_string(iss);
  EXPECT_EQ(p.s1, s);
 }

 {
  std::istringstream iss("");
  std::string s = joedb::read_string(iss);
  EXPECT_EQ("", s);
 }

 {
  std::istringstream iss("   qsdf  azer");
  std::string s = joedb::read_string(iss);
  EXPECT_EQ("qsdf", s);
 }

 {
  std::istringstream iss("\"Hello");
  std::string s = joedb::read_string(iss);
  EXPECT_EQ("Hello", s);
 }

 {
  std::istringstream iss("\"\\x0d\"");
  std::string s = joedb::read_string(iss);
  EXPECT_EQ("\x0d", s);
 }

 {
  std::istringstream iss("\"\\x z\"");
  std::string s = joedb::read_string(iss);
  EXPECT_EQ(std::string(1, 0), s);
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, write_string)
/////////////////////////////////////////////////////////////////////////////
{
 for (auto p: pairs)
 {
  std::ostringstream out;
  joedb::write_string(out, p.s1);
  EXPECT_EQ(p.s2, out.str());
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, random)
/////////////////////////////////////////////////////////////////////////////
{
 const int N = 1000;
 const int Length = 100;
 std::string s1(Length, 0);

 auto ud = std::uniform_int_distribution<>(0, 255);
 auto mt = std::mt19937(0);

 for (int j = N; --j >= 0;)
 {
  for (size_t i = 0; i < Length; i++)
   s1[i] = char(ud(mt));

  std::ostringstream out;
  joedb::write_string(out, s1);
  std::istringstream in(out.str());
  std::string s2 = joedb::read_string(in);
  ASSERT_EQ(s1, s2) << "out.str() = " << out.str();
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, sql)
/////////////////////////////////////////////////////////////////////////////
{
 std::ostringstream oss;
 joedb::write_sql_string(oss, "Rémi Coulom");
 EXPECT_EQ(oss.str(), "X'52c3a96d6920436f756c6f6d'");
}

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, base64_encode)
/////////////////////////////////////////////////////////////////////////////
{
 EXPECT_EQ("TWFu", joedb::base64_encode("Man"));
 EXPECT_EQ("TWE=", joedb::base64_encode("Ma"));
 EXPECT_EQ("TQ==", joedb::base64_encode("M"));
 EXPECT_EQ("UsOpbWk=", joedb::base64_encode("Rémi"));
 EXPECT_EQ("44GT44KM44Gv5pel5pys6Kqe44Gn44GZ", joedb::base64_encode("これは日本語です"));
 EXPECT_EQ("", joedb::base64_encode(""));
}

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, base64_decode)
/////////////////////////////////////////////////////////////////////////////
{
 EXPECT_EQ("Man", joedb::base64_decode("TWFu"));
 EXPECT_EQ("Ma", joedb::base64_decode("TWE="));
 EXPECT_EQ("M", joedb::base64_decode("TQ=="));
 EXPECT_EQ("Rémi", joedb::base64_decode("UsOpbWk="));
 EXPECT_EQ("これは日本語です", joedb::base64_decode("44GT44KM44Gv5pel5pys6Kqe44Gn44GZ"));
 EXPECT_EQ("", joedb::base64_decode(""));
}

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, base64_random)
/////////////////////////////////////////////////////////////////////////////
{
 const size_t max_size = 100;
 const int count = 10000;

 auto sd = std::uniform_int_distribution<>(0, max_size);
 auto cd = std::uniform_int_distribution<>(0, 255);
 auto mt = std::mt19937(0);

 for (int i = count; --i >= 0;)
 {
  std::string original(size_t(sd(mt)), ' ');

  for (char &c: original)
   c = char(cd(mt));

  EXPECT_EQ(original, joedb::base64_decode(joedb::base64_encode(original)));
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, base64_random_decode)
/////////////////////////////////////////////////////////////////////////////
{
 const size_t max_size = 100;
 const int count = 10000;

 auto sd = std::uniform_int_distribution<>(0, max_size);
 auto cd = std::uniform_int_distribution<>(0, 255);
 auto mt = std::mt19937(0);

 for (int i = count; --i >= 0;)
 {
  std::string garbage(size_t(sd(mt)), ' ');

  for (char &c: garbage)
   c = char(cd(mt));

  joedb::base64_decode(garbage);
 }
}

// based on http://www.zedwood.com/article/cpp-is-valid-utf8-string-function
/////////////////////////////////////////////////////////////////////////////
static bool check_utf8(const std::string& string)
/////////////////////////////////////////////////////////////////////////////
{
 const size_t size = string.size();

 for (size_t i = 0; i < size; i++)
 {
  int n;
  int c = (unsigned char)string[i];

  if (0x00 <= c && c <= 0x7f)
  {
   n = 0;  // 0bbbbbbb
  }
  else if ((c & 0xE0) == 0xC0)
  {
   n = 1;  // 110bbbbb
  }
  else if
  (
   c == 0xed &&
   i + 1 < size &&
   ((unsigned char)string[i + 1] & 0xa0) == 0xa0
  )
  {
   return false;  // U+d800 to U+dfff
  }
  else if ((c & 0xF0) == 0xE0)
  {
   n = 2;  // 1110bbbb
  }
  else if ((c & 0xF8) == 0xF0)
  {
   n = 3;  // 11110bbb
  }
  // else if (($c & 0xFC) == 0xF8) n=4; // 111110bb //byte 5, unnecessary in 4
  // byte UTF-8 else if (($c & 0xFE) == 0xFC) n=5; // 1111110b //byte 6,
  // unnecessary in 4 byte UTF-8
  else
  {
   return false;
  }
  for (int j = 0; j < n && i < size; j++)
  {  // n bytes matching 10bbbbbb follow ?
   ++i;
   if ((i == size) || (((unsigned char)string[i] & 0xC0) != 0x80))
   {
    return false;
   }
  }
 }

 return true;
}

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, utf8_validation)
/////////////////////////////////////////////////////////////////////////////
{
 const size_t size = 100000;
 std::string s;
 s.resize(size);

 auto cd = std::uniform_int_distribution<>(0, 255);
 auto mt = std::mt19937(0);

 for (int i = size; --i >= 0;)
  s[i] = char(cd(mt));

 std::ostringstream out;
 joedb::write_string(out, s);
 EXPECT_TRUE(check_utf8(out.str()));
}

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, json_rejects_invalid_utf8)
/////////////////////////////////////////////////////////////////////////////
{
 const struct
 {
  const char *name;
  const char *bytes;
 } cases[] =
 {
  {"overlong two-byte NUL", "\xc0\x80"},
  {"overlong two-byte ASCII", "\xc1\xbf"},
  {"overlong three-byte NUL", "\xe0\x80\x80"},
  {"overlong three-byte character", "\xe0\x9f\xbf"},
  {"overlong four-byte NUL", "\xf0\x80\x80\x80"},
  {"overlong four-byte character", "\xf0\x8f\xbf\xbf"},
  {"first surrogate", "\xed\xa0\x80"},
  {"last surrogate", "\xed\xbf\xbf"},
  {"above U+10FFFF", "\xf4\x90\x80\x80"},
  {"invalid leading byte", "\xf5\x80\x80\x80"},
  {"stray continuation byte", "\x80"},
  {"truncated character", "\xf0\x90\x80"}
 };

 for (const auto &c: cases)
 {
  SCOPED_TRACE(c.name);
  std::ostringstream out;
  EXPECT_THROW(joedb::write_string(out, c.bytes, true), joedb::Exception);
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, json_accepts_utf8_boundaries)
/////////////////////////////////////////////////////////////////////////////
{
 const char * const cases[] =
 {
  "\xc2\x80",         // U+0080
  "\xdf\xbf",         // U+07FF
  "\xe0\xa0\x80",     // U+0800
  "\xed\x9f\xbf",     // U+D7FF, before surrogates
  "\xee\x80\x80",     // U+E000, after surrogates
  "\xef\xbf\xbf",     // U+FFFF
  "\xf0\x90\x80\x80", // U+10000
  "\xf4\x8f\xbf\xbf"  // U+10FFFF
 };

 for (const char *s: cases)
 {
  SCOPED_TRACE(s);
  std::ostringstream out;
  EXPECT_NO_THROW(joedb::write_string(out, s, true));
  EXPECT_EQ('"' + std::string(s) + '"', out.str());
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, trigraphs)
/////////////////////////////////////////////////////////////////////////////
{
 std::ostringstream out;
 joedb::write_string(out, "?\?!");
 EXPECT_EQ(out.str(), "\"?\\?!\"");
}
