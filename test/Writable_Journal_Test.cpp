#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/File.h"
#include "joedb/ui/dump.h"
#include "joedb/ui/Interpreter.h"
#include "joedb/ui/Interpreter_Dump_Writable.h"
#include "joedb/Multiplexer.h"
#include "joedb/interpreted/Database.h"

#include "gtest/gtest.h"

#include <fstream>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
class Writable_Journal_Test: public ::testing::Test
/////////////////////////////////////////////////////////////////////////////
{
 protected:
  void SetUp() override
  {
   TearDown();
  }

  void TearDown() override
  {
   std::remove("test.joedb");
   std::remove("test_copy.joedb");
  }
};

/////////////////////////////////////////////////////////////////////////////
TEST_F(Writable_Journal_Test, basic_operations)
/////////////////////////////////////////////////////////////////////////////
{
 Database db1;

 {
  File file("test.joedb", Open_Mode::create_new);
  Writable_Journal journal(file);
  Multiplexer multi{db1, journal};

  multi.create_table("deleted");
  multi.drop_table(db1.find_table("deleted"));
  multi.create_table("table_test");

  {
   const Table_Id table_id = db1.find_table("table_test");
   multi.insert_into(table_id, Record_Id(1));
   multi.add_field(table_id, "field", Type::int32());
   const Field_Id field_id = db1.find_field(table_id, "field");
   multi.update_int32(table_id, Record_Id(1), field_id, 1234);
   multi.delete_from(table_id, Record_Id(1));
   multi.insert_into(table_id, Record_Id(2));
   multi.update_int32(table_id, Record_Id(2), field_id, 4567);
   multi.drop_field(table_id, field_id);

   multi.add_field(table_id, "big_field", Type::int64());
   const Field_Id big_field_id = db1.find_field(table_id, "big_field");
   multi.update_int64(table_id, Record_Id(2), big_field_id, 1234567ULL);

   multi.add_field(table_id, "new_field", Type::reference(table_id));
   const Field_Id new_field = db1.find_field(table_id, "new_field");
   multi.update_reference(table_id, Record_Id(2), new_field, Record_Id(2));
   multi.add_field(table_id, "name", Type::string());
   const Field_Id name_id = db1.find_field(table_id, "name");
   multi.update_string(table_id, Record_Id(2), name_id, "Aristide");
  }

  {
   multi.create_table("type_test");
   const Table_Id table_id = db1.find_table("type_test");
   multi.insert_into(table_id, Record_Id(1));

   multi.add_field(table_id, "string", Type::string());
   multi.add_field(table_id, "int32", Type::int32());
   multi.add_field(table_id, "int64", Type::int64());
   multi.add_field(table_id, "reference", Type::reference(table_id));
   multi.add_field(table_id, "bool", Type::boolean());
   multi.add_field(table_id, "float32", Type::float32());
   multi.add_field(table_id, "float64", Type::float64());

   const Field_Id string_field_id = db1.find_field(table_id, "string");
   const Field_Id int32_field_id = db1.find_field(table_id, "int32");
   const Field_Id int64_field_id = db1.find_field(table_id, "int64");
   const Field_Id reference_field_id = db1.find_field(table_id, "reference");
   const Field_Id bool_field_id = db1.find_field(table_id, "bool");
   const Field_Id float32_field_id = db1.find_field(table_id, "float32");
   const Field_Id float64_field_id = db1.find_field(table_id, "float64");

   multi.update_string(table_id, Record_Id(1), string_field_id, "SuperString");
   multi.update_int32(table_id, Record_Id(1), int32_field_id, 1234);
   multi.update_int64(table_id, Record_Id(1), int64_field_id, 123412341234LL);
   multi.update_reference(table_id, Record_Id(1), reference_field_id, Record_Id(1));
   multi.update_boolean(table_id, Record_Id(1), bool_field_id, true);
   multi.update_float32(table_id, Record_Id(1), float32_field_id, 3.14f);
   multi.update_float64(table_id, Record_Id(1), float64_field_id, 3.141592653589);
  }
  journal.checkpoint(Commit_Level::full_commit);
 }

 Database db2;

 {
  File file("test.joedb", Open_Mode::read_existing);
  Readonly_Journal journal(file);
  journal.replay_log(db2);
 }

 std::ostringstream oss1;
 std::ostringstream oss2;

 ui::Interpreter_Dump_Writable writable1(oss1);
 ui::Interpreter_Dump_Writable writable2(oss2);

 joedb::ui::dump(db1, writable1);
 joedb::ui::dump(db2, writable2);

 EXPECT_EQ(oss1.str(), oss2.str());
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(Writable_Journal_Test, interpreter_test)
/////////////////////////////////////////////////////////////////////////////
{
 //
 // First, write a .joedb file with the interpreter_test commands
 //
 {
  File file("test.joedb", Open_Mode::create_new);
  Writable_Journal journal(file);

  Database db;
  Writable dummy_writable;
  Multiplexer multiplexer{db, journal, dummy_writable};

  ui::Interpreter interpreter(db, multiplexer, nullptr, multiplexer, 0);
  std::ifstream in_file("interpreter_test.joedbi");
  ASSERT_TRUE(in_file.good());
  std::ostringstream out;
  interpreter.main_loop(in_file, out);
  multiplexer.default_checkpoint();
 }

 //
 // Then, replay test.joedb into test_copy.joedb
 //
 {
  File file("test.joedb", Open_Mode::read_existing);
  Readonly_Journal journal(file);

  File file_copy("test_copy.joedb", Open_Mode::create_new);
  Writable_Journal journal_copy(file_copy);

  Database db;
  Multiplexer multiplexer{db, journal_copy};
  journal.replay_log(multiplexer);
 }

 //
 // check that test.joedb and test_copy.joedb are identical
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
