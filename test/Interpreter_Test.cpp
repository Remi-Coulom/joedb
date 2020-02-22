#include "Interpreter.h"
#include "Interpreter_Dump_Writable.h"
#include "SQL_Dump_Writable.h"
#include "Database.h"
#include "gtest/gtest.h"
#include "Readable_Multiplexer.h"

#include <fstream>
#include <sstream>

using namespace joedb;

class Interpreter_Test: public::testing::Test
{
 protected:
  Database db_storage;
  Readable_Multiplexer db;
  Interpreter interpreter;

  Interpreter_Test():
   db(db_storage),
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

 EXPECT_EQ(reference_string.str(), out_string.str());
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(Interpreter_Test, Interpreter_Dump_Writable)
{
 std::ostringstream dump_string;
 joedb::Interpreter_Dump_Writable writable(dump_string);
 db.add_writable(writable);

 std::ifstream in_file("interpreter_test.joedbi");
 ASSERT_TRUE(in_file.good());
 std::ostringstream out_string;
 interpreter.main_loop(in_file, out_string);
 std::ofstream("interpreter_test.dump.tmp") << dump_string.str();

 std::ifstream reference_file("interpreter_test.dump");
 ASSERT_TRUE(reference_file.good());
 std::ostringstream reference_string;
 reference_string << reference_file.rdbuf();

 EXPECT_EQ(reference_string.str(), dump_string.str());
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(Interpreter_Test, SQL_Dump_Writable)
{
 std::ostringstream dump_string;
 joedb::SQL_Dump_Writable writable(dump_string);
 db.add_writable(writable);

 std::ifstream in_file("interpreter_test.joedbi");
 ASSERT_TRUE(in_file.good());
 std::ostringstream out_string;
 interpreter.main_loop(in_file, out_string);
 std::ofstream("interpreter_test.sql.tmp") << dump_string.str();

 std::ifstream reference_file("interpreter_test.sql");
 ASSERT_TRUE(reference_file.good());
 std::ostringstream reference_string;
 reference_string << reference_file.rdbuf();

 EXPECT_EQ(reference_string.str(), dump_string.str());
}
