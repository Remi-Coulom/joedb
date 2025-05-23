#ifndef joedb_Stream_File_declared
#define joedb_Stream_File_declared

#include "joedb/journal/Buffered_File.h"

#include <streambuf>

namespace joedb
{
 /// @ingroup journal
 class Stream_File: public Buffered_File
 {
  private:
   std::streambuf &streambuf;
   mutable std::streambuf::pos_type pos;
   void seek(int64_t offset) const;

  public:
   static constexpr bool lockable = false;

   Stream_File(std::streambuf &streambuf, Open_Mode mode);

   size_t pread(char *data, size_t size, int64_t offset) const override;
   void pwrite(const char *data, size_t size, int64_t offset) override;
   int64_t get_size() const override;

   ~Stream_File() override;
 };
}

#endif
