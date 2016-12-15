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
TEST_F(Multiplexer_Test, interpreter_test)
/////////////////////////////////////////////////////////////////////////////
{
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
  Multiplexer multiplexer;

  File file("test_copy.joedb", File::mode_t::create_new);
  Journal_File journal(file);
  multiplexer.add_writeable(journal);

  Database db;
  db.set_writeable(multiplexer);
  Interpreter interpreter(db);
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
