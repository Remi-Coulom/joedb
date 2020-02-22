#include "Selective_Writable.h"
#include "Readable_Multiplexer.h"
#include "Interpreter_Dump_Writable.h"
#include "Interpreter.h"
#include "gtest/gtest.h"

#include <fstream>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
static bool identical(const char *name_1, const char *name_2)
/////////////////////////////////////////////////////////////////////////////
{
 std::ifstream file_1(name_1);
 std::ifstream file_2(name_2);

 std::ostringstream string_1;
 std::ostringstream string_2;

 string_1 << file_1.rdbuf();
 string_2 << file_2.rdbuf();

 return string_1.str() == string_2.str() && string_1.str().size() > 0;
}

/////////////////////////////////////////////////////////////////////////////
TEST(Selective_Writable_Test, basic)
/////////////////////////////////////////////////////////////////////////////
{
 {
  std::ofstream schema_file("select_schema.out.tmp");
  std::ofstream information_file("select_information.out.tmp");
  std::ofstream data_file("select_data.out.tmp");

  Interpreter_Dump_Writable schema_writable(schema_file);
  Interpreter_Dump_Writable information_writable(information_file);
  Interpreter_Dump_Writable data_writable(data_file);

  Selective_Writable select_schema(schema_writable,
                                    Selective_Writable::Mode::schema);
  Selective_Writable select_information(information_writable,
                                    Selective_Writable::Mode::information);
  Selective_Writable select_data(data_writable,
                                    Selective_Writable::Mode::data_and_schema);

  Database db;
  Readable_Multiplexer multiplexer(db);
  multiplexer.add_writable(select_schema);
  multiplexer.add_writable(select_information);
  multiplexer.add_writable(select_data);

  Interpreter interpreter(multiplexer);
  std::ifstream in_file("interpreter_test.joedbi");
  ASSERT_TRUE(in_file.good());
  std::ostringstream out;
  interpreter.main_loop(in_file, out);
 }

 EXPECT_TRUE(identical("select_schema.out", "select_schema.out.tmp"));
 EXPECT_TRUE(identical("select_information.out", "select_information.out.tmp"));
 EXPECT_TRUE(identical("select_data.out", "select_data.out.tmp"));
}
