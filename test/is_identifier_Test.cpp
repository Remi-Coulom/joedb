#include "is_identifier.h"
#include "gtest/gtest.h"

TEST(check_identifier_Test, main_test)
{
 EXPECT_TRUE(joedb::is_identifier("Azerty"));
 EXPECT_TRUE(joedb::is_identifier("_AzerBq1234"));
 EXPECT_FALSE(joedb::is_identifier(""));
 EXPECT_FALSE(joedb::is_identifier("123"));
 EXPECT_FALSE(joedb::is_identifier("123abcd"));
}
