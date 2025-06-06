#include "joedb/ui/dump.h"
#include "joedb/ui/Interpreter.h"
#include "joedb/interpreted/Database.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Readonly_Memory_File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/ui/Interpreter_Dump_Writable.h"
#include "joedb/Multiplexer.h"

#include "gtest/gtest.h"

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(pack, simple_pack)
/////////////////////////////////////////////////////////////////////////////
{
 static char const * const joedbi =
"create_table float\n\
add_field float value float32\n\
insert_into float 0 0.5\n\
insert_into float 1 0.23\n\
insert_into float 2 0.7\n\
insert_into float 6 0.8\n\
insert_into float 7 9.0\n\
create_table toto\n\
drop_table toto\n\
";

 static char const * const packed_result =
"create_table float\n\
add_field float value float32\n\
create_table toto\n\
drop_table toto\n\
insert_vector float 0 3\n\
update_vector float 0 value 3 0.5 0.23 0.7\n\
insert_vector float 6 2\n\
update_vector float 6 value 2 0.8 9\n\
\n\
";

 Memory_File file;

 {
  Writable_Journal journal(file);

  Database db;
  Multiplexer multiplexer{db, journal};

  {
   std::istringstream joedbi_iss(joedbi);
   std::ostringstream joedbi_oss;

   Interpreter interpreter(db, multiplexer, Record_Id::null);
   interpreter.main_loop(joedbi_iss, joedbi_oss);
   multiplexer.soft_checkpoint();
  }
 }

 {
  const std::string &v = file.get_data();
  joedb::Readonly_Memory_File readonly_file(v.data(), v.size());
  Readonly_Journal journal(readonly_file);

  std::stringstream packed_ss;
  Interpreter_Dump_Writable writable(packed_ss);
  pack(journal, writable);
  EXPECT_EQ(packed_ss.str(), packed_result);
 }

 {
  Readonly_Journal input_journal(file);
  Memory_File output_file;
  Writable_Journal output_journal(output_file);
  pack(input_journal, output_journal);
 }
}
