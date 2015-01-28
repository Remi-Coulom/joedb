#include "sqlite3.h"
#include <cstdio>

static const char * const file_name = "./insert.sqlite3";

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
 if (argc <= 1)
  std::printf("usage: %s <number of rows>\n", argv[0]);
 else
 {
  int N = 0;
  std::sscanf(argv[1], "%d", &N);
  std::printf("N = %d\n", N);

  std::remove(file_name);
  sqlite3 *db;
  sqlite3_open(file_name, &db);
  sqlite3_exec(db, "CREATE TABLE BENCHMARK(NAME TEXT, VALUE INTEGER)", 0, 0, 0);
  //sqlite3_exec(db, "PRAGMA synchronous=OFF", 0, 0, 0);
  sqlite3_exec(db, "BEGIN TRANSACTION", 0, 0, 0);

  sqlite3_stmt *prepared_statement;
  sqlite3_prepare_v2(db,
                     "INSERT INTO BENCHMARK VALUES('TOTO', 18838586676582)",
                     -1,
                     &prepared_statement, 0);

  for (int i = N; --i >= 0;)
  {
   sqlite3_step(prepared_statement);
   sqlite3_reset(prepared_statement);
  }

  sqlite3_exec(db, "END TRANSACTION", 0, 0, 0);
 }

 return 0;
}
