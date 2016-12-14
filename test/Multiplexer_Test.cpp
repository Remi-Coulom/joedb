#include "Multiplexer.h"
#include "File.h"
#include "Journal_File.h"
#include "Database.h"
#include "DB_Writeable.h"
#include "Interpreter.h"
#include "Dummy_Writeable.h"
#include "Selective_Writeable.h"

#include "gtest/gtest.h"

#include <fstream>
#include <sstream>

using namespace joedb;

class Multiplexer_Test: public ::testing::Test
{
 protected:
  virtual void TearDown()
  {
   std::remove("multiplexer.joedb");
   std::remove("test.joedb");
   std::remove("test_copy.joedb");
  }
};

/////////////////////////////////////////////////////////////////////////////
TEST_F(Multiplexer_Test, basic)
/////////////////////////////////////////////////////////////////////////////
{
 static char const * const file_name = "multiplexer.joedb";

 //
 // Check synchronization of two interpreted databases
 //
 {
  File file(file_name, File::mode_t::create_new);
  EXPECT_EQ(file.get_status(), File::status_t::success);
  Database db1;
  Database db2;

  Journal_File journal(file);
  DB_Writeable db1_writeable(db1);
  DB_Writeable db2_writeable(db2);

  Multiplexer multiplexer;
  Writeable &journal_multiplexer = multiplexer.add_writeable(journal);
  Writeable &db1_multiplexer = multiplexer.add_writeable(db1_writeable);
  Writeable &db2_multiplexer = multiplexer.add_writeable(db2_writeable);

  db1.set_writeable(db1_multiplexer);
  db2.set_writeable(db2_multiplexer);

  journal.replay_log(journal_multiplexer);

  EXPECT_EQ(0ULL, db1.get_tables().size());
  EXPECT_EQ(0ULL, db2.get_tables().size());

  db1.create_table("T");

  EXPECT_EQ(1ULL, db1.get_tables().size());
  EXPECT_EQ(1ULL, db2.get_tables().size());

  db2.create_table("U");

  EXPECT_EQ(2ULL, db1.get_tables().size());
  EXPECT_EQ(2ULL, db2.get_tables().size());
 }

 //
 // Replaying log with two interpreted databases
 //
 {
  File file(file_name, File::mode_t::read_existing);
  EXPECT_EQ(file.get_status(), File::status_t::success);

  Database db1;
  Database db2;

  Journal_File journal(file);
  DB_Writeable db1_writeable(db1);
  DB_Writeable db2_writeable(db2);

  Multiplexer multiplexer;
  Writeable &journal_multiplexer = multiplexer.add_writeable(journal);
  Writeable &db1_multiplexer = multiplexer.add_writeable(db1_writeable);
  Writeable &db2_multiplexer = multiplexer.add_writeable(db2_writeable);

  db1.set_writeable(db1_multiplexer);
  db2.set_writeable(db2_multiplexer);

  journal.replay_log(journal_multiplexer);

  EXPECT_EQ(2ULL, db1.get_tables().size());
  EXPECT_EQ(2ULL, db2.get_tables().size());
 }

 std::remove(file_name);
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(Multiplexer_Test, interpreter_test)
/////////////////////////////////////////////////////////////////////////////
{
 Multiplexer multiplexer;

 //
 // Directly write to test.joedb
 //
 {
  File file("test.joedb", File::mode_t::create_new);
  Journal_File journal(file);

  Database db;
  db.set_writeable(journal);

  Interpreter interpreter(db);
  std::ifstream in_file("interpreter_test.joedbi");
  ASSERT_TRUE(in_file.good());
  std::ostringstream out;
  interpreter.main_loop(in_file, out);
 }

 //
 // Write to test_copy through the multiplexer
 //
 {
  Database db1;
  Database db2;

  DB_Writeable db1_writeable(db1);
  DB_Writeable db2_writeable(db2);

  Dummy_Writeable dummy;
  Selective_Writeable select1(dummy, Selective_Writeable::schema);
  Selective_Writeable select2(dummy, Selective_Writeable::data);
  Selective_Writeable select4(dummy, Selective_Writeable::information);
  dummy.comment("hello");

  Writeable &db1_multiplexer = multiplexer.add_writeable(db1_writeable);
                               multiplexer.add_writeable(db2_writeable);
                               multiplexer.add_writeable(select1);
                               multiplexer.add_writeable(select2);
                               multiplexer.add_writeable(select4);

  db1.set_writeable(db1_multiplexer);

  File file("test_copy.joedb", File::mode_t::create_new);
  Journal_File journal(file);
  db2.set_writeable(journal);

  Interpreter interpreter(db1);
  std::ifstream in_file("interpreter_test.joedbi");
  ASSERT_TRUE(in_file.good());
  std::ostringstream out;
  interpreter.main_loop(in_file, out);
 }

 //
 // Check that they are identical
 //
 {
  std::ifstream file("test.joedb");
  std::ostringstream bytes_of_file;
  bytes_of_file << file.rdbuf();

  std::ifstream file_copy("test_copy.joedb");
  std::ostringstream bytes_of_file_copy;
  bytes_of_file_copy << file_copy.rdbuf();

  EXPECT_EQ(bytes_of_file.str(), bytes_of_file_copy.str());
 }
}
