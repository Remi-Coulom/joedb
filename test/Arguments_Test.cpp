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
  EXPECT_EQ
  (
   arguments.get_string_option("socket", "endpoint_path", "joedb.sock"),
   "joedb.sock"
  );
  EXPECT_EQ(arguments.get_option<float>("timeout", "seconds", 0.0f), 1.23f);
 }

 TEST(Arguments, empty)
 {
  const std::vector<const char *> v{"", "", ""};
  Arguments arguments(int(v.size()), v.data());
 }

 TEST(Arguments, option_defaults)
 {
  const std::vector<const char *> labels{"first", "second"};
  Arguments empty;
  EXPECT_EQ(empty.get_string_option("name", "value", "default"), "default");
  EXPECT_EQ(empty.get_option<int>("count", "value", 42), 42);
  EXPECT_EQ(empty.get_enum_option("mode", labels, 1), 1);

  const char *argv[] = {"prog", "--name"};
  Arguments missing_value(2, argv);
  EXPECT_EQ(missing_value.get_string_option("name", "value", "default"), "default");
  EXPECT_EQ(missing_value.get_next(), "--name");
 }

 TEST(Arguments, option_values_and_positionals)
 {
  const char *argv[] = {"prog", "--name", "", "positional", "--mode", "second"};
  const std::vector<const char *> labels{"first", "second"};
  Arguments arguments(6, argv);
  EXPECT_EQ(arguments.get_string_option("name", "value", "default"), "");
  EXPECT_EQ(arguments.get_enum_option("mode", labels, 0), 1);
  EXPECT_EQ(arguments.get_next(), "positional");
  EXPECT_FALSE(arguments.has_error());

  const char *invalid_argv[] = {"prog", "--mode", "invalid"};
  Arguments invalid(3, invalid_argv);
  try
  {
   invalid.get_enum_option("mode", labels, 0);
   FAIL() << "Expected an invalid option value to throw";
  }
  catch (const std::exception &e)
  {
   EXPECT_STREQ(e.what(), "invalid value for option --mode: invalid");
  }
 }

 TEST(Arguments, print_help)
 {
  const std::vector<const char *> v
  {
   "./args_test",
   "arg1",
   "--follow",
   "--timeout",
   "1.23"
  };

  Arguments arguments(int(v.size()), v.data());
  EXPECT_EQ(arguments.get_next("<argument>"), "arg1");
  EXPECT_EQ(arguments.get_option<float>("timeout", "seconds", 0.0f), 1.23f);
  EXPECT_EQ
  (
   arguments.get_string_option("socket", "endpoint_path", "joedb.sock"),
   "joedb.sock"
  );

  arguments.add_parameter("--follow");
  std::ostringstream out;
  arguments.print_help(out);
  EXPECT_EQ
  (
   out.str(),
   "usage: ./args_test <argument> "
   "[--timeout <seconds>] "
   "[--socket <endpoint_path>] "
   "--follow\n"
  );
 }

 TEST(Arguments, next_option)
 {
  const std::vector<const char *> v{"prog", "--keep_alive", "1.23"};
  Arguments arguments(int(v.size()), v.data());
  EXPECT_EQ(1.23f, arguments.next_option<float>("keep_alive", "", 2.0f));
  EXPECT_EQ(2.0f, arguments.next_option<float>("keep_alive", "", 2.0f));
 }

 TEST(Arguments, next_option_order)
 {
  const std::vector<const char *> v{"prog", "azerty", "--keep_alive", "1.23"};
  Arguments arguments(int(v.size()), v.data());
  EXPECT_EQ(2.0f, arguments.next_option<float>("keep_alive", "", 2.0f));
 }

 TEST(Arguments, string)
 {
  const std::vector<const char *> v{"prog", "--words", "hello words"};

  {
   Arguments arguments(int(v.size()), v.data());
   EXPECT_EQ("hello words", arguments.get_option<std::string>("words", "description", "toto"));
  }

  {
   Arguments arguments(int(v.size()), v.data());
   EXPECT_EQ("hello words", arguments.next_option<std::string>("words", "description", "toto"));
  }
 }
} // namespace joedb
