#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/ui/dump.h"
#include "joedb/ui/Interpreter.h"
#include "joedb/ui/Interpreter_Dump_Writable.h"
#include "joedb/Multiplexer.h"
#include "joedb/interpreted/Database.h"

#include "gtest/gtest.h"

#include <fstream>
#include <array>
#include <cstring>
#include <limits>
#include <random>

using namespace joedb;

namespace
{
 class Copying_Database: public Database
 {
  #define TYPE_MACRO(cpp_type, return_type, type_id, R, W)\
  cpp_type *get_own_##type_id##_storage(Table_Id, Record_Id, Field_Id, size_t &capacity) override\
  {capacity = 0; return nullptr;}
  #include "joedb/TYPE_MACRO.h"
 };

 template<typename T> struct Numeric_Journal_IO;
 #define TYPE_MACRO(cpp_type, return_type, type_id, R, W)\
 template<> struct Numeric_Journal_IO<cpp_type>\
 {\
  static Type type() {return Type::type_id();}\
  static void scalar(Writable &w, Table_Id t, Record_Id r, Field_Id f, cpp_type v)\
  {w.update_##type_id(t, r, f, v);}\
  static void vector(Writable &w, Table_Id t, Record_Id r, Field_Id f, size_t n, const cpp_type *v)\
  {w.update_vector_##type_id(t, r, f, n, v);}\
  static cpp_type get(const Database &db, Table_Id t, Record_Id r, Field_Id f)\
  {return db.get_##type_id(t, r, f);}\
 };
 #define TYPE_MACRO_NO_STRING
 #define TYPE_MACRO_NO_REFERENCE
 #define TYPE_MACRO_NO_BOOL
 #define TYPE_MACRO_NO_BLOB
 #include "joedb/TYPE_MACRO.h"

 template<typename T> class Numeric_Journal_Test: public testing::Test {};
 using Numeric_Types = testing::Types<int8_t, int16_t, int32_t, int64_t, float, double>;
 TYPED_TEST_SUITE(Numeric_Journal_Test, Numeric_Types);

 TYPED_TEST(Numeric_Journal_Test, scalar_and_vector_bit_patterns)
 {
  using T = TypeParam;
  using IO = Numeric_Journal_IO<T>;
  Memory_File file;
  Writable_Journal writer(file);
  std::array<std::array<std::array<T, 32>, 2>, 2> expected{};
  for (const std::string name: {"first", "second"})
   writer.create_table(name);
  for (int table = 0; table < 2; ++table)
  {
   const Table_Id tid{uint16_t(table + 1)};
   writer.add_field(tid, "a", IO::type());
   writer.add_field(tid, "b", IO::type());
   writer.insert_vector(tid, Record_Id{0}, 32);
  }
  std::mt19937_64 random(0x80f01);
  const auto check = [&](const Database &db)
  {
   for (int table = 0; table < 2; ++table)
    for (int field = 0; field < 2; ++field)
     for (int record = 0; record < 32; ++record)
     {
      const T actual = IO::get(db, Table_Id{uint16_t(table + 1)}, Record_Id{record}, Field_Id{uint16_t(field + 1)});
      ASSERT_EQ(std::memcmp(&actual, &expected[table][field][record], sizeof(T)), 0)
       << "table=" << table << " field=" << field << " record=" << record;
     }
  };
  for (size_t step = 0; step < 300; ++step)
  {
   SCOPED_TRACE(step);
   const size_t table = random() % 2;
   const size_t field = random() % 2;
   const size_t count = 1 + random() % 8;
   const size_t start = random() % (33 - count);
   std::array<T, 8> values;
   for (auto &value: values)
   {
    const uint64_t bits = random();
    std::memcpy(&value, &bits, sizeof(T));
   }
   values[0] = step % 2 ? std::numeric_limits<T>::lowest() : std::numeric_limits<T>::max();
   if (step % 2)
    IO::vector(writer, Table_Id{uint16_t(table + 1)}, Record_Id{index_t(start)},
     Field_Id{uint16_t(field + 1)}, count, values.data());
   else
    for (size_t i = 0; i < count; ++i)
     IO::scalar(writer, Table_Id{uint16_t(table + 1)}, Record_Id{index_t(start + i)},
      Field_Id{uint16_t(field + 1)}, values[i]);
   std::copy_n(values.begin(), count, expected[table][field].begin() + start);
   if (step % 41 == 0 || step == 299)
   {
    writer.soft_checkpoint();
    Database direct;
    Readonly_Journal reader(file);
    reader.replay_log(direct);
    ASSERT_NO_FATAL_FAILURE(check(direct));
    Copying_Database fallback;
    reader.replay_log(fallback);
    ASSERT_NO_FATAL_FAILURE(check(fallback));
   }
  }
 }
}

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
   multi.update_float32(table_id, Record_Id(1), float32_field_id, 1.23f);
   multi.update_float64(table_id, Record_Id(1), float64_field_id, 1.23456789012345);
  }
  journal.hard_checkpoint();
 }

 Database db2;

 {
  File file("test.joedb", Open_Mode::read_existing);
  Readonly_Journal journal(file);
  journal.replay_log(db2);
 }

 std::ostringstream oss1;
 std::ostringstream oss2;

 Interpreter_Dump_Writable writable1(oss1);
 Interpreter_Dump_Writable writable2(oss2);

 joedb::dump(db1, writable1);
 joedb::dump(db2, writable2);

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
  Dummy_Writable dummy_writable;
  Multiplexer multiplexer{db, journal, dummy_writable};

  Interpreter interpreter(db, multiplexer, Record_Id::null);
  std::ifstream in_file("interpreter_test.joedbi");
  ASSERT_TRUE(in_file.good());
  std::ostringstream out;
  interpreter.main_loop(in_file, out);
  multiplexer.soft_checkpoint();
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

  EXPECT_TRUE(journal_copy.equal_to(journal, Header::ssize, journal.get_checkpoint()));
 }
}
