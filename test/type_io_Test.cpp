#include "type_io.h"
#include "gtest/gtest.h"

#include <sstream>

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
