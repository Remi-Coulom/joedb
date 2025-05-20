#ifndef joedb_File_Slice_declared
#define joedb_File_Slice_declared

#include "joedb/journal/Readonly_Memory_File.h"
#include "joedb/journal/Posix_File.h"

#include <sys/mman.h>

namespace joedb
{
 namespace detail
 {
  class Memory_Mapping
  {
   private:
    void * const data;
    const size_t size;

   public:
    Memory_Mapping(int fd, size_t size):
     data(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0)),
     size(size)
    {
     if (data == MAP_FAILED)
      Posix_File::throw_last_error("mapping", "file");
    }

    Memory_Mapping(const Memory_Mapping &) = delete;
    Memory_Mapping &operator=(const Memory_Mapping &) = delete;

    const char *get() const {return (const char *)data;}

    ~Memory_Mapping()
    {
     munmap(data, size);
    }
  };
 }

 /// @ingroup journal
 class File_Slice: public detail::Memory_Mapping, public Readonly_Memory_File
 {
  public:
   File_Slice(int fd, size_t offset, size_t size):
    detail::Memory_Mapping(fd, offset + size),
    Readonly_Memory_File(detail::Memory_Mapping::get() + offset, size)
   {
   }
 }; 
}

#endif
