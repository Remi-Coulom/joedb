#include "Journal_File.h"
#include "File.h"
#include "gtest/gtest.h"
#include "dump.h"
#include "Interpreter.h"
#include "Interpreter_Dump_Writeable.h"
#include "Readable_Multiplexer.h"

#include <fstream>

using namespace joedb;

class Journal_File_Test: public ::testing::Test
{
 protected:
  virtual void TearDown()
  {
   std::remove("test.joedb");
   std::remove("test_copy.joedb");
  }
};

/////////////////////////////////////////////////////////////////////////////
TEST_F(Journal_File_Test, bad_file)
/////////////////////////////////////////////////////////////////////////////
{
 File file("this_does_not_exists", File::mode_t::read_existing);
 EXPECT_EQ(file.get_status(), joedb::File::status_t::failure);
 Journal_File journal(file);
 EXPECT_EQ(Journal_File::state_t::bad_file, journal.get_state());
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(Journal_File_Test, basic_operations)
/////////////////////////////////////////////////////////////////////////////
{
 Database db1;

 {
  File file("test.joedb", File::mode_t::create_new);
  Journal_File journal(file);
  Readable_Multiplexer multi(db1);
  multi.add_writeable(journal);

  multi.create_table("deleted");
  multi.drop_table(db1.find_table("deleted"));
  multi.create_table("table_test");
  const table_id_t table_id = multi.find_table("table_test");
  multi.insert_into(table_id, 1);
  multi.add_field(table_id, "field", Type::int32());
  const field_id_t field_id = multi.find_field(table_id, "field");
  multi.update_int32(table_id, 1, field_id, 1234);
  multi.delete_from(table_id, 1);
  multi.insert_into(table_id, 2);
  multi.update_int32(table_id, 2, field_id, 4567);
  multi.drop_field(table_id, field_id);

  multi.add_field(table_id, "big_field", Type::int64());
  const field_id_t big_field_id = multi.find_field(table_id, "big_field");
  multi.update_int64(table_id, 2, big_field_id, 1234567ULL);

  multi.add_field(table_id, "new_field", Type::reference(table_id));
  const field_id_t new_field = multi.find_field(table_id, "new_field");
  multi.update_reference(table_id, 2, new_field, 2);
  multi.add_field(table_id, "name", Type::string());
  const field_id_t name_id = multi.find_field(table_id, "name");
  multi.update_string(table_id, 2, name_id, "Aristide");

  {
   multi.create_table("type_test");
   const table_id_t table_id = multi.find_table("type_test");
   multi.insert_into(table_id, 1);

   multi.add_field(table_id, "string", Type::string());
   multi.add_field(table_id, "int32", Type::int32());
   multi.add_field(table_id, "int64", Type::int64());
   multi.add_field(table_id, "reference", Type::reference(table_id));
   multi.add_field(table_id, "bool", Type::boolean());
   multi.add_field(table_id, "float32", Type::float32());
   multi.add_field(table_id, "float64", Type::float64());

   const field_id_t string_field_id = multi.find_field(table_id, "string");
   const field_id_t int32_field_id = multi.find_field(table_id, "int32");
   const field_id_t int64_field_id = multi.find_field(table_id, "int64");
   const field_id_t reference_field_id = multi.find_field(table_id, "reference");
   const field_id_t bool_field_id = multi.find_field(table_id, "bool");
   const field_id_t float32_field_id = multi.find_field(table_id, "float32");
   const field_id_t float64_field_id = multi.find_field(table_id, "float64");

   multi.update_string(table_id, 1, string_field_id, "SuperString");
   multi.update_int32(table_id, 1, int32_field_id, 1234);
   multi.update_int64(table_id, 1, int64_field_id, 123412341234LL);
   multi.update_reference(table_id, 1, reference_field_id, 1);
   multi.update_boolean(table_id, 1, bool_field_id, true);
   multi.update_float32(table_id, 1, float32_field_id, 3.14f);
   multi.update_float64(table_id, 1, float64_field_id, 3.141592653589);
  }
  journal.checkpoint(2);
 }

 Database db2;

 {
  File file("test.joedb", File::mode_t::read_existing);
  Journal_File journal(file);
  journal.replay_log(db2);
  EXPECT_EQ(Journal_File::state_t::no_error, journal.get_state());
 }

 std::ostringstream oss1;
 std::ostringstream oss2;

 Interpreter_Dump_Writeable writeable1(oss1);
 Interpreter_Dump_Writeable writeable2(oss2);

 joedb::dump(db1, writeable1);
 joedb::dump(db2, writeable2);

 EXPECT_EQ(oss1.str(), oss2.str());
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(Journal_File_Test, interpreter_test)
/////////////////////////////////////////////////////////////////////////////
{
 //
 // First, write a .joedb file with the interpreter_test commands
 //
 {
  File file("test.joedb", File::mode_t::create_new);
  Journal_File journal(file);

  Database db_storage;
  Readable_Multiplexer db(db_storage);
  db.add_writeable(journal);

  Interpreter interpreter(db);
  std::ifstream in_file("interpreter_test.joedbi");
  ASSERT_TRUE(in_file.good());
  std::ostringstream out;
  interpreter.main_loop(in_file, out);
 }

 //
 // Then, replay test.joedb into test_copy.joedb
 //
 {
  File file("test.joedb", File::mode_t::read_existing);
  Journal_File journal(file);

  File file_copy("test_copy.joedb", File::mode_t::create_new);
  Journal_File journal_copy(file_copy);

  Database db_storage;
  Readable_Multiplexer db(db_storage);
  db.add_writeable(journal_copy);
  journal.replay_log(db);
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
