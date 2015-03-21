#include "utf8.h"
#include "gtest/gtest.h"

#include <sstream>

// TODO: find an utf-8 character that contains a " inside

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
 {u8"\"\n\\\t\x1f"    , u8"\"\\\"\\n\\\\\\t\\x1f\""},
 {u8"これは日本語です", u8"\"これは日本語です\""},
 {u8"𩸽"              , u8"\"𩸽\""}, // 4-byte character
 {u8""                , u8"\"\""}
};

TEST(UTF8_Test, write_hexa_character)
{
 std::ostringstream out;
 joedb::write_hexa_character(out, 0x1a);
 EXPECT_EQ(out.str(), "\\x1a");
}

TEST(UTF8_Test, read_utf8_string)
{
 for (auto p: pairs)
 {
  std::istringstream iss(p.s2);
  std::string s = joedb::read_utf8_string(iss);
  EXPECT_EQ(s, p.s1);
 }

 {
  std::istringstream iss("Hello");
  std::string s = joedb::read_utf8_string(iss);
  EXPECT_EQ(s, "Hello");
 }
}

TEST(UTF8_Test, write_utf8_string)
{
 for (auto p: pairs)
 {
  std::ostringstream out;
  joedb::write_utf8_string(out, p.s1);
  EXPECT_EQ(out.str(), p.s2);
 }
}
