#include "gtest/gtest.h"
#include "joedb/compiler/nested_namespace.h"

#include <sstream>

namespace joedb::compiler
{
 ////////////////////////////////////////////////////////////////////////////
 TEST(nested_namespace, split_namespace)
 ////////////////////////////////////////////////////////////////////////////
 {
  auto n = split_namespace("split::this::name");

  EXPECT_EQ(3, int(n.size()));
  EXPECT_EQ("split", n[0]);
  EXPECT_EQ("this", n[1]);
  EXPECT_EQ("name", n[2]);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(nested_namespace, namespace_write)
 ////////////////////////////////////////////////////////////////////////////
 {
  auto n = split_namespace("split::this::name");
  std::ostringstream out;
  namespace_write(out, n, "!");
  EXPECT_EQ("split!this!name", out.str());
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(nested_namespace, namespace_string)
 ////////////////////////////////////////////////////////////////////////////
 {
  auto n = split_namespace("split::this::name");
  EXPECT_EQ("split..this..name", namespace_string(n, ".."));
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(nested_namespace, namespace_open)
 ////////////////////////////////////////////////////////////////////////////
 {
  auto n = split_namespace("split::this::name");
  std::ostringstream out;
  namespace_open(out, n);
  EXPECT_EQ("namespace split::this::name\n{", out.str());
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(nested_namespace, namespace_close)
 ////////////////////////////////////////////////////////////////////////////
 {
  auto n = split_namespace("split::this::name");
  std::ostringstream out;
  namespace_close(out, n);
  EXPECT_EQ("}\n", out.str());
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(nested_namespace, namespace_include_guard)
 ////////////////////////////////////////////////////////////////////////////
 {
  auto n = split_namespace("split::this::name");
  std::ostringstream out;
  namespace_include_guard_open(out, "X", n);
  EXPECT_EQ
  (
   "#ifndef split_this_name_X_declared\n#define split_this_name_X_declared\n",
   out.str()
  );
 }
}
