#include "joedb/ui/type_io.h"

#include "gtest/gtest.h"

#include <sstream>

TEST(type_io_Test, invalid_boolean_sets_failure)
{
 for (const char *text: {"", "invalid", "TRUE", "2"})
 {
  std::istringstream input(text);
  EXPECT_FALSE(joedb::read_boolean(input));
  EXPECT_TRUE(input.fail());
 }
}

TEST(type_io_Test, primitive_io)
{
 {
  std::istringstream iss("500");
  int32_t i = joedb::read_int32(iss);
  EXPECT_EQ(i, 500);
  EXPECT_FALSE(iss.fail());
 }

 {
  std::istringstream iss("\"500\"");
  int32_t i = joedb::read_int32(iss);
  EXPECT_EQ(i, 0);
  EXPECT_TRUE(iss.fail());
 }
}

TEST(type_io_Test, int8_parse_failure)
{
 for (const char *input: {"", "invalid"})
 {
  std::istringstream in(input);
  EXPECT_EQ(joedb::read_int8(in), 0);
  EXPECT_TRUE(in.fail());
 }
}

TEST(type_io_Test, unquoted_string_at_eof)
{
 std::istringstream in("Alice");
 EXPECT_EQ(joedb::read_string(in), "Alice");
 EXPECT_TRUE(in.eof());
 EXPECT_FALSE(in.fail());
}

TEST(type_io_Test, incomplete_strings)
{
 for (const char *input: {"", "\"", "\"unfinished", "prefix\\", "prefix\\x", "prefix\\x4", "prefix\\1", "prefix\\12"})
 {
  SCOPED_TRACE(input);
  std::istringstream in(input);
  joedb::read_string(in);
  EXPECT_TRUE(in.fail());
 }
}
