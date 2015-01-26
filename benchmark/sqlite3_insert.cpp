#include "sqlite3.h"
#include <cstdio>

static const char * const file_name = "./insert.sqlite3";

/////////////////////////////////////////////////////////////////////////////
void quick_exec(sqlite3 *db, const char *statement)
{
 sqlite3_stmt *prepared_statement;
 sqlite3_prepare_v2(db, statement, -1, &prepared_statement, 0);
 sqlite3_step(prepared_statement);
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
 std::remove(file_name);
 sqlite3 *db;
 sqlite3_open(file_name, &db);

 quick_exec(db, "CREATE TABLE BENCHMARK(NAME TEXT, VALUE INTEGER)");

 if (argc <= 1)
  std::printf("usage: %s <number of rows>\n", argv[0]);
 else
 {
  int N = 0;
  std::sscanf(argv[1], "%d", &N);
  std::printf("N = %d\n", N);

//  quick_exec(db, "PRAGMA synchronous=OFF");
  quick_exec(db, "BEGIN TRANSACTION");

  sqlite3_stmt *prepared_statement;
  sqlite3_prepare_v2(db,
   "INSERT INTO BENCHMARK VALUES('TOTO', 18838586676582)", -1, &prepared_statement, 0);

  for (int i = N; --i >= 0;)
  {
   //quick_exec(db, "INSERT INTO BENCHMARK VALUES('TOTO')");
   sqlite3_step(prepared_statement);
  }

  quick_exec(db, "END TRANSACTION");
 }

 return 0;
}
