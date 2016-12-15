#include <cstdio>
#include "joedb/File.h"
#include "joedb/Journal_File.h"
#include "joedb/Database.h"
#include "joedb/Readable_Multiplexer.h"

using namespace joedb;

static const char * const file_name = "./insert.joedb";

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 std::remove(file_name);

 if (argc <= 1)
  std::printf("usage: %s <number of rows>\n", argv[0]);
 else
 {
  int N = 0;
  std::sscanf(argv[1], "%d", &N);
  std::printf("N = %d\n", N);

  File file(file_name, File::mode_t::create_new);
  Journal_File journal_file(file);
  Database db;
  Readable_Multiplexer multiplexer(db);
  multiplexer.add_writeable(journal_file);

  multiplexer.create_table("BENCHMARK");
  table_id_t table_id = multiplexer.find_table("BENCHMARK");
  multiplexer.add_field(table_id, "NAME", Type::string());
  field_id_t name_id = multiplexer.find_field(table_id, "NAME");
  multiplexer.add_field(table_id, "VALUE", Type::int64());
  field_id_t value_id = multiplexer.find_field(table_id, "VALUE");

  const std::string name_string("TOTO");
  for (int i = 1; i <= N; i++)
  {
   const record_id_t record_id = record_id_t(i);
   multiplexer.insert(table_id, record_id);
   multiplexer.update_string(table_id, record_id, name_id, name_string);
   multiplexer.update_int64(table_id, record_id, value_id, i);
  }

  journal_file.checkpoint(2);
 }

 return 0;
}
