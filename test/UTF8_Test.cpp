#include "utf8.h"
#include "gtest/gtest.h"

#include <sstream>

TEST(UTF8_Test, write_hexa_character)
{
 std::ostringstream out;
 joedb::write_hexa_character(out, 0x1a);
 EXPECT_EQ(out.str(), "\\x1a");
}

TEST(UTF8_Test, write_utf8_string)
{
 {
  std::ostringstream out;
  joedb::write_utf8_string(out, "Hello, world!", false);
  EXPECT_EQ(out.str(), "\"Hello, world!\"");
 }

 {
  std::ostringstream out;
  joedb::write_utf8_string(out, "\"\n\\\t\x1f");
  EXPECT_EQ(out.str(), "\"\\\"\\n\\\\t\\x1f\"");
 }

 {
  std::ostringstream out;
  joedb::write_utf8_string(out, u8"Rémi");
  EXPECT_EQ(out.str(), u8"\"Rémi\"");
 }

 {
  std::ostringstream out;
  joedb::write_utf8_string(out, u8"これは日本語です");
  EXPECT_EQ(out.str(), u8"\"これは日本語です\"");
 }

 // 4-byte character
 {
  std::ostringstream out;
  joedb::write_utf8_string(out, u8"𩸽");
  EXPECT_EQ(out.str(), u8"\"𩸽\"");
 }
}
