#ifndef joedb_File_Slice_declared
#define joedb_File_Slice_declared

#include "joedb/journal/Readonly_Memory_File.h"
#include "joedb/journal/Posix_File.h"

#include <sys/mman.h>

namespace joedb
{
 class File_Slice: public Readonly_Memory_File
 {
  private:
   const size_t mapped_size;

  public:
   File_Slice(int fd, size_t offset, size_t size):
    Readonly_Memory_File
    (
     (char *)mmap
     (
      nullptr,
      offset + size,
      PROT_READ,
      MAP_PRIVATE,
      fd,
      0
     ) + offset,
     size
    ),
    mapped_size(offset + size)
   {
    if (data - offset == MAP_FAILED)
     Posix_File::throw_last_error("mapping", "file");
   }

   ~File_Slice()
   {
    munmap((void *)data, mapped_size);
   }
 }; 
}

#endif
