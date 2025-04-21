#include "joedb/Multiplexer.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/interpreted/Database.h"
#include "joedb/ui/Interpreter.h"
#include "joedb/ui/Interpreter_Dump_Writable.h"
#include "joedb/ui/dump.h"
#include "joedb/journal/Memory_File.h"

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
  Interpreter interpreter(reference_db, reference_db, nullptr, reference_db, 0);
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
  Multiplexer multiplexer{multiplexed_db};
  Interpreter interpreter(multiplexed_db, multiplexer, nullptr, multiplexer, 0);
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

/////////////////////////////////////////////////////////////////////////////
TEST(Multiplexer_Test, flush)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File f1;
 joedb::Memory_File f2;

 joedb::Writable_Journal j1(f1);
 joedb::Writable_Journal j2(f2);

 joedb::Multiplexer multiplexer{j1, j2};

 EXPECT_EQ(f1.get_size(), 41);
 EXPECT_EQ(f2.get_size(), 41);

 multiplexer.comment("Hello");

 EXPECT_EQ(f1.get_size(), 41);
 EXPECT_EQ(f2.get_size(), 41);

 multiplexer.flush();

 EXPECT_EQ(f1.get_size(), 48);
 EXPECT_EQ(f2.get_size(), 48);

 EXPECT_EQ(j1.get_checkpoint_position(), 41);
 EXPECT_EQ(j2.get_checkpoint_position(), 41);

 multiplexer.soft_checkpoint();

 EXPECT_EQ(j1.get_checkpoint_position(), 48);
 EXPECT_EQ(j2.get_checkpoint_position(), 48);
}
