#include "File.h"
#include <cstdio>

static const char * const file_name = "file_benchmark.bin";

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc <= 1)
 {
  std::printf("usage: %s <N>\n", argv[0]);
  return 1;
 }
 else
 {
  int N = 0;
  std::sscanf(argv[1], "%d", &N);
  std::printf("N = %d\n", N);

  {
   std::remove(file_name);
   joedb::File file(file_name, joedb::Open_Mode::create_new);

   for (int i = N; --i >= 0;)
    file.write<int64_t>(i);
  }

  {
   joedb::File file(file_name, joedb::Open_Mode::read_existing);
   for (int i = N; --i >= 0;)
   {
    const int64_t n = file.read<int64_t>();
    if (n != i)
    {
     std::printf("Error!\n");
     return 1;
    }
   }
  }
 }

 return 0;
}
