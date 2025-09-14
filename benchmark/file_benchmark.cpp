#include "joedb/journal/File.h"
#include "joedb/journal/File_Buffer.h"

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
   joedb::File_Buffer file_buffer(file);

   for (int i = N; --i >= 0;)
    file_buffer.write<int64_t>(i);

   file_buffer.flush();
  }

  {
   joedb::File file(file_name, joedb::Open_Mode::read_existing);
   joedb::File_Buffer file_buffer(file);

   for (int i = N; --i >= 0;)
   {
    const int64_t n = file_buffer.read<int64_t>();
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
