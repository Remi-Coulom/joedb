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

  const std::string s("TOTO");

  for (int i = 1; i <= N; i++)
   db.new_BENCHMARK(s, i);

  db.checkpoint();
  db.commit();
 }

 return 0;
}
