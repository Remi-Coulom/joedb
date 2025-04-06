#ifndef joedb_Async_Reader_declared
#define joedb_Async_Reader_declared

#include "joedb/journal/Abstract_File.h"
#include "joedb/Blob.h"

namespace joedb
{
 /// @ingroup journal
 class Async_Reader
 {
  private:
   const Abstract_File &file;
   int64_t end;
   int64_t current;
   bool end_of_file;

  public:
   Async_Reader(const Abstract_File &file, int64_t start, int64_t end);
   Async_Reader(const Abstract_File &file, Blob blob);

   size_t read(char *buffer, size_t capacity);

   bool is_end_of_file() const {return end_of_file;}
   int64_t get_end() const {return end;}
   int64_t get_current() const {return current;}
   int64_t get_remaining() const {return end - current;}
 };
}

#endif
