#ifndef joedb_File_Slice_declared
#define joedb_File_Slice_declared

#include "joedb/journal/Portable_File.h"

namespace joedb
{
 ///////////////////////////////////////////////////////////////////////////
 class File_Slice: public Portable_File
 ///////////////////////////////////////////////////////////////////////////
 {
  public:
   File_Slice(FILE *file, size_t start, size_t length):
    Portable_File(file, Open_Mode::read_existing),
    start(start),
    length(length)
   {
    seek(0);
   }

   ~File_Slice() override {}

  protected:
   int64_t get_size() const override
   {
    return int64_t(length);
   }

   int seek(int64_t offset) override
   {
    return Portable_File::seek(offset + int64_t(start));
   }

  private:
   const size_t start;
   const size_t length;
 };
}

#endif
