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
 if (argc <= 1)
  std::printf("usage: %s <number of rows>\n", argv[0]);
 else
 {
  size_t N = 0;
  std::sscanf(argv[1], "%zu", &N);
  std::printf("N = %zu\n", N);

  File file(file_name, Open_Mode::create_new);
  Journal_File journal_file(file);
  Database db;
  Readable_Multiplexer multiplexer(db);
  multiplexer.add_writable(journal_file);

  multiplexer.create_table("BENCHMARK");
  Table_Id table_id = multiplexer.find_table("BENCHMARK");
  multiplexer.add_field(table_id, "NAME", Type::string());
  Field_Id name_id = multiplexer.find_field(table_id, "NAME");
  multiplexer.add_field(table_id, "VALUE", Type::int64());
  Field_Id value_id = multiplexer.find_field(table_id, "VALUE");

  const std::string name_string("TOTO");

#if 1
  for (size_t i = 1; i <= N; i++)
  {
   multiplexer.insert_into(table_id, i);
   multiplexer.update_string(table_id, i, name_id, name_string);
   multiplexer.update_int64(table_id, i, value_id, int64_t(i));
  }
#else
  multiplexer.insert_vector(table_id, 1, N);
  for (size_t i = 1; i <= N; i++)
   multiplexer.update_string(table_id, i, name_id, name_string);
  for (size_t i = 1; i <= N; i++)
   multiplexer.update_int64(table_id, i, value_id, int64_t(i));
#endif

  journal_file.checkpoint(2);
 }

 return 0;
}
