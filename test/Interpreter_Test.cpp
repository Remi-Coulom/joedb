#include "joedb/ui/Interpreter.h"
#include "joedb/ui/Interpreter_Dump_Writable.h"
#include "joedb/ui/SQL_Dump_Writable.h"
#include "joedb/ui/Raw_Dump_Writable.h"
#include "joedb/ui/Blob_Reader_Command_Processor.h"
#include "joedb/interpreted/Database.h"
#include "joedb/journal/Interpreted_File.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/concurrency/Writable_Database_Client.h"
#include "joedb/Multiplexer.h"
#include "gtest/gtest.h"

#include <fstream>
#include <sstream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 TEST(Interpreter_Test, main_test)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;

  std::remove("bio.txt.tmp");

  {
   Writable_Journal journal(file);
   Database db;
   Multiplexer multiplexer{db, journal};
   Interpreter interpreter(db, multiplexer, Record_Id::null);
   Blob_Reader_Command_Processor blob_processor(file);
   interpreter.add_processor(blob_processor);

   std::ifstream in_file("interpreter_test.joedbi");
   ASSERT_TRUE(in_file);
   std::ostringstream out_string;
   interpreter.main_loop(in_file, out_string);
   std::ofstream("interpreter_test.out.tmp") << out_string.str();

   std::ifstream reference_file("interpreter_test.out");
   ASSERT_TRUE(reference_file);
   std::ostringstream reference_string;
   reference_string << reference_file.rdbuf();

   EXPECT_EQ(reference_string.str(), out_string.str());
   journal.soft_checkpoint();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Interpreter_Test, Interpreter_Dump_Writable)
 ////////////////////////////////////////////////////////////////////////////
 {
  Database db;
  Memory_File file;
  Writable_Journal journal(file);
  std::ostringstream dump_string;
  const bool blob_wanted = true;
  Interpreter_Dump_Writable writable(dump_string, blob_wanted);
  Multiplexer multiplexer{db, journal, writable};
  Interpreter interpreter(db, multiplexer, Record_Id::null);

  std::ifstream in_file("interpreter_test.joedbi");
  ASSERT_TRUE(in_file);
  std::ostringstream out_string;
  interpreter.main_loop(in_file, out_string);
  std::ofstream("interpreter_test.dump.tmp") << dump_string.str();

  std::ifstream reference_file("interpreter_test.dump");
  ASSERT_TRUE(reference_file);
  std::ostringstream reference_string;
  reference_string << reference_file.rdbuf();

  EXPECT_EQ(reference_string.str(), dump_string.str());
  journal.soft_checkpoint();
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Interpreter_Test, SQL_Dump_Writable)
 ////////////////////////////////////////////////////////////////////////////
 {
  Database db;
  Memory_File file;
  Writable_Journal journal(file);
  std::ostringstream dump_string;
  SQL_Dump_Writable writable(dump_string, &file);
  writable.on_blob(Blob());
  Multiplexer multiplexer{db, journal, writable};
  Interpreter interpreter(db, multiplexer, Record_Id::null);

  std::ifstream in_file("interpreter_test.joedbi");
  ASSERT_TRUE(in_file);
  std::ostringstream out_string;
  interpreter.main_loop(in_file, out_string);
  std::ofstream("interpreter_test.sql.tmp") << dump_string.str();

  std::ifstream reference_file("interpreter_test.sql");
  ASSERT_TRUE(reference_file);
  std::ostringstream reference_string;
  reference_string << reference_file.rdbuf();

  EXPECT_EQ(reference_string.str(), dump_string.str());
  journal.soft_checkpoint();
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Interpreter_Test, Raw_Dump_Writable)
 ////////////////////////////////////////////////////////////////////////////
 {
  Database db;
  std::ostringstream dump_string;
  Raw_Dump_Writable writable(dump_string);
  Multiplexer multiplexer{db, writable};
  Interpreter interpreter(db, multiplexer, Record_Id::null);

  std::ifstream in_file("interpreter_test.joedbi");
  ASSERT_TRUE(in_file);
  std::ostringstream out_string;
  interpreter.main_loop(in_file, out_string);
  std::ofstream("interpreter_test.raw.tmp") << dump_string.str();

  std::ifstream reference_file("interpreter_test.raw");
  ASSERT_TRUE(reference_file);
  std::ostringstream reference_string;
  reference_string << reference_file.rdbuf();

  EXPECT_EQ(reference_string.str(), dump_string.str());
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Interpreter, Readonly_Interpreted_File)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::stringstream ss;
  Readonly_Interpreted_File file(ss);
  EXPECT_ANY_THROW(Writable_Journal{file});
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Interpreter, Interpreted_File)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::stringstream ss;
  ss << "create_table person\n";
  ss << "insert_into person -1\n";
  ss << "insert_into person -1\n";
  ss << "insert_into person -1\n";
  ss << "create_table city\n";

  Readonly_Interpreted_File file(ss);
  Readonly_Journal journal(file);
  Database db;
  journal.play_until_checkpoint(db);
  EXPECT_EQ(db.get_tables().size(), 2ULL);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Interpreter, Writable_Interpreted_File)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File memory_file;
  streambuf buf(memory_file);
  std::iostream stream(&buf);

  {
   Interpreted_Stream_File file(stream);
   Writable_Journal journal(file);
   journal.rewind();
   journal.create_table("person");
   journal.create_table("city");
   journal.soft_checkpoint();
   journal.insert_into(Table_Id{1}, Record_Id{0});
   journal.soft_checkpoint();
  }

  EXPECT_EQ(memory_file.get_data(), "create_table person\ncreate_table city\n\ninsert_into person 0\n\n");

  {
   Interpreted_Stream_File file(stream);
   Writable_Journal journal(file);
   Database writable;
   journal.play_until_checkpoint(writable);
   journal.comment("Hello");
   journal.soft_checkpoint();
  }

  EXPECT_EQ(memory_file.get_data(), "create_table person\ncreate_table city\n\ninsert_into person 0\n\ncomment \"Hello\"\n\n");

  Readonly_Interpreted_File file(stream);
  Readonly_Journal journal(file);
  Database db;
  journal.play_until_checkpoint(db);
  EXPECT_EQ(db.get_tables().size(), 2ULL);
  EXPECT_EQ(db.get_tables().begin()->first, Table_Id{1});
  EXPECT_EQ((++db.get_tables().begin())->first, Table_Id{2});
  EXPECT_EQ(db.get_freedom(Table_Id{1}).size(), 1);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Interpreter, double_insert)
 ////////////////////////////////////////////////////////////////////////////
 {
  // Use the interpreter to write a double "insert_into person 1"
  Memory_File file;
  Writable_Database_Client client(file);
  client.transaction([](const Readable &readable, Writable &writable){
   Interpreter interpreter(readable, writable, Record_Id::null);
   std::fstream null_stream;
   std::istringstream iss
   (
    "create_table person\n"
    "insert_into person 1\n"
    "insert_into person 1\n"
   );
   interpreter.main_loop(iss, null_stream);
  });

  // It is better to catch the error before writing to the file
  // The second erroneous insert should not be in the file
  {
   Readonly_Journal journal(file);
   Database db;
   journal.replay_log(db);
  }
 }
}
