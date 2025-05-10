#include "joedb/ui/Arguments.h"

#include "gtest/gtest.h"

namespace joedb
{
 TEST(Arguments, options)
 {
  const std::vector<const char *> v{"--follow", "--timeout", "1.23"};
  Arguments arguments(int(v.size()), v.data());
  EXPECT_TRUE(arguments.has_flag("follow"));
  EXPECT_FALSE(arguments.has_flag("qsdf"));
  EXPECT_EQ(arguments.get_string_option("timeout", "seconds", "0"), "1.23");
  EXPECT_EQ(arguments.get_string_option("socket", "endpoint_path", "joedb.sock"), "joedb.sock");
  EXPECT_EQ(arguments.get_option<float>("timeout", "seconds", 0.0f), 1.23f);
 }
}
