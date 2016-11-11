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

  benchmark::File_Database db("insert.joedb");

  const std::string toto = "TOTO";

  for (int i = 1; i <= N; i++)
   db.new_benchmark(toto, i);

  db.checkpoint_full_commit();
 }

 return 0;
}
