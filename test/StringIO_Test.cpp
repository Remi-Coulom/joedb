#include "type_io.h"
#include "base64.h"
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
 {u8"Hello"           , u8"\"Hello\""},
 {u8"Rémi"            , u8"\"Rémi\""},
 {u8"Hello, world!"   , u8"\"Hello, world!\""},
 {u8"\"\\\x1f"        , u8"\"\\\"\\\\\\037\""},
 {u8"これは日本語です", u8"\"これは日本語です\""},
 {u8"𩸽"              , u8"\"𩸽\""}, // 4-byte character
 {u8""                , u8"\"\""},
 {u8"4"               , u8"\"4\""}
};

/////////////////////////////////////////////////////////////////////////////
TEST(StringIO_Test, utf8_display_size)
/////////////////////////////////////////////////////////////////////////////
{
 EXPECT_EQ(size_t(0), joedb::utf8_display_size(u8""));
 EXPECT_EQ(size_t(4), joedb::utf8_display_size(u8"Remi"));
 EXPECT_EQ(size_t(4), joedb::utf8_display_size(u8"Rémi"));
 EXPECT_EQ(size_t(4), joedb::utf8_display_size(u8"山下"));
 EXPECT_EQ(size_t(2), joedb::utf8_display_size(u8"𩸽"));
 EXPECT_EQ(size_t(4), joedb::utf8_display_size(u8"바둑"));
 EXPECT_EQ(size_t(4), joedb::utf8_display_size(u8"圍棋"));
 EXPECT_EQ(size_t(4), joedb::utf8_display_size(u8"围棋"));
 EXPECT_EQ(size_t(4), joedb::utf8_display_size(u8"囲碁"));
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
TEST(StringIO_Test, base64)
/////////////////////////////////////////////////////////////////////////////
{
 EXPECT_EQ("TWFu", joedb::base64_encode("Man"));
 EXPECT_EQ("TWE=", joedb::base64_encode("Ma"));
 EXPECT_EQ("TQ==", joedb::base64_encode("M"));
 EXPECT_EQ("UsOpbWk=", joedb::base64_encode("Rémi"));
 EXPECT_EQ("44GT44KM44Gv5pel5pys6Kqe44Gn44GZ", joedb::base64_encode("これは日本語です"));
 EXPECT_EQ("", joedb::base64_encode(""));
}
