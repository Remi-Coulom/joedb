#include "string_io.h"
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
 {u8"\"\\\x1f"        , u8"\"\\\"\\\\\\x1f\""},
 {u8"これは日本語です", u8"\"これは日本語です\""},
 {u8"𩸽"              , u8"\"𩸽\""}, // 4-byte character
 {u8""                , u8"\"\""}
};

TEST(StringIO_Test, write_hexa_character)
{
 std::ostringstream out;
 joedb::write_hexa_character(out, 0x1a);
 EXPECT_EQ("\\x1a", out.str());
}

TEST(StringIO_Test, read_string)
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
  std::istringstream iss("\"\\x z\"");
  std::string s = joedb::read_string(iss);
  EXPECT_EQ(std::string(1, 0), s);
 }
}

TEST(StringIO_Test, write_string)
{
 for (auto p: pairs)
 {
  std::ostringstream out;
  joedb::write_string(out, p.s1);
  EXPECT_EQ(p.s2, out.str());
 }
}

TEST(StringIO_Test, random)
{
 const int N = 1000;
 const int Length = 100;
 std::string s1(Length, 0);

 auto ud = std::uniform_int_distribution<>(0, 255);
 auto mt = std::mt19937(0);

 for (int j = N; --j >= 0;)
 {
  for (int i = 0; i < Length; i++)
   s1[i] = char(ud(mt));

  std::ostringstream out;
  joedb::write_string(out, s1);
  std::istringstream in(out.str());
  std::string s2 = joedb::read_string(in);
  ASSERT_EQ(s1, s2) << "out.str() = " << out.str();
 }
}
