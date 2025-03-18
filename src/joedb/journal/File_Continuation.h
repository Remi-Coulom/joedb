#ifndef joedb_File_Continuation_declared
#define joedb_File_Continuation_declared

#include "joedb/journal/Generic_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class File_Continuation: public Generic_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Generic_File &file;
   Generic_File &continuation;
   const int64_t continuation_offset;

  protected:
   void raw_sync() override;
   void shared_lock(int64_t start, int64_t size) override;
   void exclusive_lock(int64_t start, int64_t size) override;
   void unlock(int64_t start, int64_t size) override;

  public:
   File_Continuation
   (
    Generic_File &file,
    Generic_File &continuation,
    int64_t continuation_offset
   );

   size_t pread(char *data, size_t size, int64_t offset) override;
   void pwrite(const char *data, size_t size, int64_t offset) override;

   int64_t get_continuation_offset() const
   {
    return continuation_offset;
   }
 };
};

#endif
