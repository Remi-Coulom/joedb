#include "benchmark.h"
#include <cstdio>

int main(int argc, char **argv)
{
 if (argc <= 1)
  std::printf("usage: %s <number of rows>\n", argv[0]);
 else
 {
  size_t N = 0;
  std::sscanf(argv[1], "%lu", &N);
  std::printf("N = %lu\n", N);

  benchmark::File_Database db("insert.joedb");

  const std::string toto = "TOTO";

#if 1
  for (size_t i = 1; i <= N; i++)
   db.new_benchmark(toto, int64_t(i));
#else
  auto v = db.new_vector_of_benchmark(N);
  for (size_t i = 0; i < N; i++)
   db.set_value(v[i], int64_t(i + 1));
  for (size_t i = 0; i < N; i++)
   db.set_name(v[i], toto);
#endif

  db.checkpoint_full_commit();
 }

 return 0;
}
