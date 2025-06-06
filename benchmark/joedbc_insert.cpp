#include "benchmarkdb/File_Database.h"
#include <cstdio>

int main(int argc, char **argv)
{
 if (argc <= 1)
  std::printf("usage: %s <number of rows>\n", argv[0]);
 else
 {
  size_t N = 0;
  std::sscanf(argv[1], "%zu", &N);
  std::printf("N = %zu\n", N);

  benchmarkdb::File_Database db("insert.joedb");

#if 0
  for (size_t i = 1; i <= N; i++)
  {
   db.new_benchmark("TOTO", int64_t(i));
   db.soft_checkpoint();
  }
#else
  {
   auto v = db.new_vector_of_benchmark(N);

   db.update_vector_of_name(v, N, [N](joedb::Span<std::string> name)
   {
    for (size_t i = 0; i < N; i++)
     name[i] = "TOTO";
   });

   db.update_vector_of_value(v, N, [N](joedb::Span<int64_t> value)
   {
    for (size_t i = 0; i < N; i++)
     value[i] = int64_t(i) + 1;
   });
  }
  db.hard_checkpoint();
#endif

 }

 return 0;
}
