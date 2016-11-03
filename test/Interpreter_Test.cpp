#include "Interpreter.h"
#include "Database.h"
#include "gtest/gtest.h"

#include <fstream>
#include <sstream>

using namespace joedb;

class Interpreter_Test: public::testing::Test
{
 protected:
  Database db;
  Interpreter interpreter;

  Interpreter_Test():
   interpreter(db)
  {
  }
};

/////////////////////////////////////////////////////////////////////////////
TEST_F(Interpreter_Test, main_test)
{
 std::ifstream in_file("interpreter_test.joedbi");
 ASSERT_TRUE(in_file.good());
 std::ostringstream out_string;
 interpreter.main_loop(in_file, out_string);
 std::ofstream("interpreter_test.out.tmp") << out_string.str();

 std::ifstream reference_file("interpreter_test.out");
 ASSERT_TRUE(reference_file.good());
 std::ostringstream reference_string;
 reference_string << reference_file.rdbuf();

 EXPECT_EQ(out_string.str(), reference_string.str());
}
