#include "Multiplexer.h"
#include "File.h"
#include "Journal_File.h"
#include "Database.h"
#include "DB_Listener.h"
#include "Interpreter.h"

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
  DB_Listener db1_listener(db1);
  DB_Listener db2_listener(db2);

  Multiplexer multiplexer;
  Listener &journal_multiplexer = multiplexer.add_listener(journal);
  Listener &db1_multiplexer = multiplexer.add_listener(db1_listener);
  Listener &db2_multiplexer = multiplexer.add_listener(db2_listener);

  db1.set_listener(db1_multiplexer);
  db2.set_listener(db2_multiplexer);

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
  DB_Listener db1_listener(db1);
  DB_Listener db2_listener(db2);

  Multiplexer multiplexer;
  Listener &journal_multiplexer = multiplexer.add_listener(journal);
  Listener &db1_multiplexer = multiplexer.add_listener(db1_listener);
  Listener &db2_multiplexer = multiplexer.add_listener(db2_listener);

  db1.set_listener(db1_multiplexer);
  db2.set_listener(db2_multiplexer);

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
  db.set_listener(journal);

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

  DB_Listener db1_listener(db1);
  DB_Listener db2_listener(db2);

  Listener &db1_multiplexer = multiplexer.add_listener(db1_listener);
                              multiplexer.add_listener(db2_listener);

  db1.set_listener(db1_multiplexer);

  File file("test_copy.joedb", File::mode_t::create_new);
  Journal_File journal(file);
  db2.set_listener(journal);

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
