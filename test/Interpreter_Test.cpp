#include "joedb/io/Interpreter.h"
#include "joedb/io/Interpreter_Dump_Writable.h"
#include "joedb/io/SQL_Dump_Writable.h"
#include "joedb/io/Raw_Dump_Writable.h"
#include "joedb/interpreter/Database.h"
#include "joedb/journal/Interpreted_File.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/Multiplexer.h"
#include "gtest/gtest.h"

#include <fstream>
#include <sstream>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(Interpreter_Test, main_test)
/////////////////////////////////////////////////////////////////////////////
{
 Database db;
 Interpreter interpreter(db, db, nullptr, nullptr, 0);

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
TEST(Interpreter_Test, Interpreter_Dump_Writable)
/////////////////////////////////////////////////////////////////////////////
{
 Database db;
 std::ostringstream dump_string;
 joedb::Interpreter_Dump_Writable writable(dump_string);
 Multiplexer multiplexer{db, writable};
 Interpreter interpreter(db, multiplexer, nullptr, nullptr, 0);

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
TEST(Interpreter_Test, SQL_Dump_Writable)
/////////////////////////////////////////////////////////////////////////////
{
 Database db;
 std::ostringstream dump_string;
 joedb::SQL_Dump_Writable writable(dump_string);
 Multiplexer multiplexer{db, writable};
 Interpreter interpreter(db, multiplexer, nullptr, nullptr, 0);

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

/////////////////////////////////////////////////////////////////////////////
TEST(Interpreter_Test, Raw_Dump_Writable)
/////////////////////////////////////////////////////////////////////////////
{
 Database db;
 std::ostringstream dump_string;
 joedb::Raw_Dump_Writable writable(dump_string);
 Multiplexer multiplexer{db, writable};
 Interpreter interpreter(db, multiplexer, nullptr, nullptr, 0);

 std::ifstream in_file("interpreter_test.joedbi");
 ASSERT_TRUE(in_file.good());
 std::ostringstream out_string;
 interpreter.main_loop(in_file, out_string);
 std::ofstream("interpreter_test.raw.tmp") << dump_string.str();

 std::ifstream reference_file("interpreter_test.raw");
 ASSERT_TRUE(reference_file.good());
 std::ostringstream reference_string;
 reference_string << reference_file.rdbuf();

 EXPECT_EQ(reference_string.str(), dump_string.str());
}

/////////////////////////////////////////////////////////////////////////////
TEST(Interpreter, Interpreted_File)
/////////////////////////////////////////////////////////////////////////////
{
 std::stringstream ss;
 ss << "create_table person\n";
 ss << "insert_into person 0\n";
 ss << "insert_into person 0\n";
 ss << "insert_into person 0\n";
 ss << "create_table city\n";
 ss.seekg(0);

 joedb::Interpreted_File file(ss);
 joedb::Readonly_Journal journal(file);
 Database db;
 journal.play_until_checkpoint(db);
 EXPECT_EQ(db.get_tables().size(), 2ULL);
}
