#include <cstdio>
#include "File.h"
#include "JournalFile.h"
#include "Database.h"

using namespace joedb;

static const char * const file_name = "./insert.joedb";

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
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
  JournalFile journal_file(file);
  Database db;
  db.set_listener(journal_file);

  table_id_t table_id = db.create_table("BENCHMARK");
  field_id_t field_id = db.add_field(table_id, "NAME", Type::string());

  std::string value("TOTO");
  for (int i = 1; i <= N; i++)
  {
   const record_id_t record_id = record_id_t(i);
   db.insert_into(table_id, record_id);
   db.update_string(table_id, record_id, field_id, value);
   //file.commit();
   //journal_file.checkpoint();
   //file.commit();
  }

  file.commit();
 }

 return 0;
}
