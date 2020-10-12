#include "gtest/gtest.h"

#include "joedb/compiler/nested_namespace.h"

/////////////////////////////////////////////////////////////////////////////
TEST(nested_namespace, splitting)
/////////////////////////////////////////////////////////////////////////////
{
 auto n = joedb::split_namespace("split::this::name");

 EXPECT_EQ(3, int(n.size()));
 EXPECT_EQ("split", n[0]);
 EXPECT_EQ("this", n[1]);
 EXPECT_EQ("name", n[2]);
}
