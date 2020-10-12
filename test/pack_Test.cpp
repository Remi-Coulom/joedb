#include "joedb/io/dump.h"
#include "joedb/io/Interpreter.h"
#include "joedb/interpreter/Database.h"
#include "joedb/journal/Stream_File.h"
#include "joedb/journal/Journal_File.h"
#include "joedb/io/Interpreter_Dump_Writable.h"
#include "joedb/Readable_Multiplexer.h"

#include "gtest/gtest.h"

#include <fstream>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(pack, simple_pack)
/////////////////////////////////////////////////////////////////////////////
{
 static char const * const joedbi =
"create_table float\n\
add_field float value float32\n\
insert_into float 1 0.5\n\
insert_into float 2 0.23\n\
insert_into float 3 0.7\n\
insert_into float 7 0.8\n\
insert_into float 8 9.0\n\
create_table toto\n\
drop_table toto\n\
";

 static char const * const packed_result =
"create_table float\n\
add_field float value float32\n\
create_table toto\n\
drop_table toto\n\
insert_vector float 1 3\n\
update_vector float 1 value 3 0.5 0.23 0.7\n\
insert_vector float 7 2\n\
update_vector float 7 value 2 0.8 9\n\
";

 std::stringstream joedb_ss;

 {
  Stream_File joedb_file(joedb_ss, Open_Mode::create_new);
  Journal_File journal(joedb_file);

  Database db;
  Readable_Multiplexer multiplexer(db);
  multiplexer.add_writable(journal);

  {
   std::istringstream joedbi_iss(joedbi);
   std::ostringstream joedbi_oss;

   Interpreter interpreter(multiplexer);
   interpreter.main_loop(joedbi_iss, joedbi_oss);
  }
 }

 {
  std::stringstream joedb_ss_bis(joedb_ss.str());
  Stream_File joedb_file(joedb_ss_bis, Open_Mode::read_existing);
  Readonly_Journal journal(joedb_file);

  std::stringstream packed_ss;
  Interpreter_Dump_Writable writable(packed_ss);
  pack(journal, writable);
  EXPECT_EQ(packed_ss.str(), packed_result);
 }
}
