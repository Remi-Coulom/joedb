#include "File.h"
#include "Journal_File.h"
#include "Interpreter.h"
#include "Safe_Listener.h"

#include "gtest/gtest.h"

#include <fstream>
#include <sstream>

using namespace joedb;

class Safe_Listener_Test: public ::testing::Test
{
 protected:
  virtual void TearDown()
  {
   std::remove("test.joedb");
   std::remove("test_copy.joedb");
  }
};

/////////////////////////////////////////////////////////////////////////////
TEST_F(Safe_Listener_Test, interpreter_test)
/////////////////////////////////////////////////////////////////////////////
{
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
 // Write to test_copy through the safe listener
 //
 {
  File output_file("test_copy.joedb", File::mode_t::create_new);
  Journal_File output_journal(output_file);

  File input_file("test.joedb", File::mode_t::read_existing);
  Journal_File input_journal(input_file);

  Safe_Listener safe_listener(output_journal, 0);
  input_journal.replay_log(safe_listener);
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
