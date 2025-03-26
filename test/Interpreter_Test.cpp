#include "joedb/ui/Interpreter.h"
#include "joedb/ui/Interpreter_Dump_Writable.h"
#include "joedb/ui/SQL_Dump_Writable.h"
#include "joedb/ui/Raw_Dump_Writable.h"
#include "joedb/interpreted/Database.h"
#include "joedb/journal/Interpreted_File.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/Multiplexer.h"
#include "gtest/gtest.h"

#include <fstream>
#include <sstream>

namespace joedb::ui
{
 /////////////////////////////////////////////////////////////////////////////
 TEST(Interpreter_Test, main_test)
 /////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;

  std::remove("bio.txt.tmp");

  {
   Writable_Journal journal(file);
   interpreted::Database db;
   Multiplexer multiplexer{db, journal};
   Interpreter interpreter(db, multiplexer, &journal, multiplexer, 0);

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
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Interpreter_Test, Interpreter_Dump_Writable)
 /////////////////////////////////////////////////////////////////////////////
 {
  interpreted::Database db;
  Memory_File file;
  Writable_Journal journal(file);
  std::ostringstream dump_string;
  const bool blob_wanted = true;
  Interpreter_Dump_Writable writable(dump_string, blob_wanted);
  Multiplexer multiplexer{db, journal, writable};
  Interpreter interpreter(db, multiplexer, &journal, multiplexer, 0);

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
  interpreted::Database db;
  Memory_File file;
  Writable_Journal journal(file);
  std::ostringstream dump_string;
  SQL_Dump_Writable writable(dump_string);
  writable.on_blob(Blob(), file);
  Multiplexer multiplexer{db, journal, writable};
  Interpreter interpreter(db, multiplexer, &journal, multiplexer, 0);

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
  interpreted::Database db;
  std::ostringstream dump_string;
  Raw_Dump_Writable writable(dump_string);
  Multiplexer multiplexer{db, writable};
  Interpreter interpreter(db, multiplexer, nullptr, multiplexer, 0);

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
 TEST(Interpreter, Readonly_Interpreted_File)
 /////////////////////////////////////////////////////////////////////////////
 {
  std::stringstream ss;
  joedb::Readonly_Interpreted_File file(ss);
  EXPECT_ANY_THROW(joedb::Writable_Journal{file});
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

  joedb::Readonly_Interpreted_File file(ss);
  joedb::Readonly_Journal journal(file);
  interpreted::Database db;
  journal.play_until_checkpoint(db);
  EXPECT_EQ(db.get_tables().size(), 2ULL);
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Interpreter, Writable_Interpreted_File)
 /////////////////////////////////////////////////////////////////////////////
 {
  std::stringstream ss;

  {
   joedb::Interpreted_Stream_File file(ss);
   joedb::Writable_Journal journal(file);
   journal.rewind();
   journal.create_table("person");
   journal.create_table("city");
   journal.default_checkpoint();
   journal.insert_into(Table_Id{1}, Record_Id{1});
   journal.default_checkpoint();
  }

  EXPECT_EQ(ss.str(), "create_table person\ncreate_table city\n\ninsert_into person 1\n");

  joedb::Readonly_Interpreted_File file(ss);
  joedb::Readonly_Journal journal(file);
  interpreted::Database db;
  journal.play_until_checkpoint(db);
  EXPECT_EQ(db.get_tables().size(), 2ULL);
  EXPECT_EQ(db.get_tables().begin()->first, Table_Id{1});
  EXPECT_EQ((++db.get_tables().begin())->first, Table_Id{2});
  EXPECT_EQ(db.get_freedom(Table_Id{1}).size(), 1);
 }
}
