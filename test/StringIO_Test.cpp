#include "joedb/io/type_io.h"
#include "joedb/io/base64.h"
#include "external/wide_char_display_width.h"

#include "gtest/gtest.h"

#include <sstream>
#include <random>

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
   if ((++i == size) || (((unsigned char)string[i] & 0xC0) != 0x80))
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
