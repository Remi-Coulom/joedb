#include "gtest/gtest.h"
#include "joedb/compiler/nested_namespace.h"

#include <sstream>

/////////////////////////////////////////////////////////////////////////////
TEST(nested_namespace, split_namespace)
/////////////////////////////////////////////////////////////////////////////
{
 auto n = joedb::split_namespace("split::this::name");

 EXPECT_EQ(3, int(n.size()));
 EXPECT_EQ("split", n[0]);
 EXPECT_EQ("this", n[1]);
 EXPECT_EQ("name", n[2]);
}

/////////////////////////////////////////////////////////////////////////////
TEST(nested_namespace, namespace_write)
/////////////////////////////////////////////////////////////////////////////
{
 auto n = joedb::split_namespace("split::this::name");
 std::ostringstream out;
 joedb::namespace_write(out, n, "!");
 EXPECT_EQ("split!this!name", out.str());
}
