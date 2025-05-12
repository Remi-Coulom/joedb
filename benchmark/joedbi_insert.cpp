#include <cstdio>
#include "joedb/journal/File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/interpreted/Database.h"
#include "joedb/Multiplexer.h"

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
  joedb::index_t N = 0;
  std::sscanf(argv[1], "%zu", &N);
  std::printf("N = %zu\n", N);

  File file(file_name, Open_Mode::create_new);
  Writable_Journal journal_file(file);
  Database db;
  Multiplexer multiplexer{db, journal_file};

  multiplexer.create_table("BENCHMARK");
  Table_Id table_id = db.find_table("BENCHMARK");
  multiplexer.add_field(table_id, "NAME", Type::string());
  Field_Id name_id = db.find_field(table_id, "NAME");
  multiplexer.add_field(table_id, "VALUE", Type::int64());
  Field_Id value_id = db.find_field(table_id, "VALUE");

  const std::string name_string("TOTO");

#if 1
  for (joedb::Record_Id i{0}; index_t(i) < N; ++i)
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

  journal_file.hard_checkpoint();
 }

 return 0;
}
