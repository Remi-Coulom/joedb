#include "Journal_File.h"
#include "File.h"
#include "DB_Listener.h"
#include "gtest/gtest.h"
#include "dump.h"
#include "Interpreter.h"
#include "Interpreter_Dump_Listener.h"

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
  db1.set_listener(journal);
  db1.drop_table(db1.create_table("deleted"));
  const table_id_t table_id = db1.create_table("table_test");
  db1.insert_into(table_id, 1);
  const field_id_t field_id = db1.add_field(table_id, "field", Type::int32());
  db1.update_int32(table_id, 1, field_id, 1234);
  db1.delete_from(table_id, 1);
  db1.insert_into(table_id, 2);
  db1.update_int32(table_id, 2, field_id, 4567);
  db1.drop_field(table_id, field_id);

  const field_id_t big_field_id = db1.add_field(table_id, "big_field", Type::int64());
  db1.update_int64(table_id, 2, big_field_id, 1234567ULL);

  const field_id_t new_field =
   db1.add_field(table_id, "new_field", Type::reference(table_id));
  db1.update_reference(table_id, 2, new_field, 2);
  const field_id_t name_id = db1.add_field(table_id, "name", Type::string());
  db1.update_string(table_id, 2, name_id, "Aristide");

  {
   const table_id_t table_id = db1.create_table("type_test");
   db1.insert_into(table_id, 1);

   const field_id_t string_field_id =
    db1.add_field(table_id, "string", Type::string());
   const field_id_t int32_field_id =
    db1.add_field(table_id, "int32", Type::int32());
   const field_id_t int64_field_id =
    db1.add_field(table_id, "int64", Type::int64());
   const field_id_t reference_field_id =
    db1.add_field(table_id, "reference", Type::reference(table_id));
   const field_id_t bool_field_id =
    db1.add_field(table_id, "bool", Type::boolean());
   const field_id_t float32_field_id =
    db1.add_field(table_id, "float32", Type::float32());
   const field_id_t float64_field_id =
    db1.add_field(table_id, "float64", Type::float64());

   db1.update_string(table_id, 1, string_field_id, "SuperString");
   db1.update_int32(table_id, 1, int32_field_id, 1234);
   db1.update_int64(table_id, 1, int64_field_id, 123412341234LL);
   db1.update_reference(table_id, 1, reference_field_id, 1);
   db1.update_boolean(table_id, 1, bool_field_id, true);
   db1.update_float32(table_id, 1, float32_field_id, 3.14f);
   db1.update_float64(table_id, 1, float64_field_id, 3.141592653589);
  }
  journal.checkpoint(2);
 }

 Database db2;

 {
  File file("test.joedb", File::mode_t::read_existing);
  Journal_File journal(file);
  DB_Listener db_listener(db2);
  journal.replay_log(db_listener);
  EXPECT_TRUE(db_listener.is_good());
  EXPECT_EQ(Journal_File::state_t::no_error, journal.get_state());
 }

 std::ostringstream oss1;
 std::ostringstream oss2;

 Interpreter_Dump_Listener listener1(oss1);
 Interpreter_Dump_Listener listener2(oss2);

 joedb::dump(db1, listener1);
 joedb::dump(db2, listener2);

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

  Database db;
  db.set_listener(journal);

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

  Database db;
  db.set_listener(journal_copy);

  DB_Listener db_listener(db);
  journal.replay_log(db_listener);
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
