#include "benchmark.h"

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

  benchmark::Database db("insert.joedb");

  if (!db.is_good())
  {
   std::printf("Error opening database\n");
   return 1;
  }

  const std::string name_string("TOTO");
  for (int i = 1; i <= N; i++)
  {
   auto record = db.new_BENCHMARK();
   record.set_NAME(db, name_string);
   record.set_VALUE(db, 18838586676582);
  }

  db.commit();
 }

 return 0;
}
