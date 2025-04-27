#include "sqlite3.h"
#include <cstdio>

int main(int argc, char **argv)
{
 if (argc <= 1)
  std::printf("usage: %s <number of rows>\n", argv[0]);
 else
 {
  int N = 0;
  std::sscanf(argv[1], "%d", &N);
  std::printf("N = %d\n", N);

  std::remove("insert.sqlite3");
  sqlite3 *db;
  sqlite3_open("insert.sqlite3", &db);
  sqlite3_exec
  (
   db,
   "CREATE TABLE BENCHMARK(NAME TEXT, VALUE INTEGER)",
   nullptr,
   nullptr,
   nullptr
  );

  sqlite3_exec(db, "PRAGMA journal_mode=WAL", nullptr, nullptr, nullptr);
  sqlite3_exec(db, "PRAGMA synchronous=NORMAL", nullptr, nullptr, nullptr);

  sqlite3_stmt *prepared_statement;
  sqlite3_prepare_v2
  (
   db,
   "INSERT INTO BENCHMARK VALUES('TOTO', ?1)",
   -1,
   &prepared_statement,
   nullptr
  );


  for (int i = 1; i <= N; i++)
  {
   sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
   sqlite3_bind_int64(prepared_statement, 1, i);
   sqlite3_step(prepared_statement);
   sqlite3_reset(prepared_statement);
   sqlite3_exec(db, "END TRANSACTION", nullptr, nullptr, nullptr);
  }

 }

 return 0;
}
