#include "Readable_Multiplexer.h"
#include "File.h"
#include "Journal_File.h"
#include "Database.h"
#include "Interpreter.h"
#include "Interpreter_Dump_Writable.h"
#include "dump.h"

#include "gtest/gtest.h"

#include <fstream>
#include <sstream>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(Multiplexer_Test, interpreter_test)
/////////////////////////////////////////////////////////////////////////////
{
 //
 // Directly read database
 //
 Database reference_db;
 {
  Interpreter interpreter(reference_db);
  std::ifstream in_file("interpreter_test.joedbi");
  ASSERT_TRUE(in_file.good());
  std::ostringstream out;
  interpreter.main_loop(in_file, out);
 }

 //
 // Read through a multiplexer
 //
 Database multiplexed_db;
 {
  Readable_Multiplexer multiplexer(multiplexed_db);

  Interpreter interpreter(multiplexer);
  std::ifstream in_file("interpreter_test.joedbi");
  ASSERT_TRUE(in_file.good());
  std::ostringstream out;
  interpreter.main_loop(in_file, out);
 }

 //
 // Check that they are identical
 //
 {
  std::ostringstream reference;
  std::ostringstream multiplexed;

  {
   Interpreter_Dump_Writable dump_writable(reference);
   dump(reference_db, dump_writable);
  }

  {
   Interpreter_Dump_Writable dump_writable(multiplexed);
   dump(multiplexed_db, dump_writable);
  }

  EXPECT_EQ(reference.str(), multiplexed.str());
 }
}
