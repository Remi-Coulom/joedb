#ifndef joedb_JournalFile_declared
#define joedb_JournalFile_declared

#include "File.h"

namespace joedb
{
 class File;

 class JournalFile
 {
  public:
   enum class state_t {no_error,
                       unsupported_version,
                       bad_format,
                       crash_check};

  private:
   static const uint32_t version_number;
   static const int64_t header_size;

   File &file;
   int checkpoint_index;
   state_t state;

  public:
   JournalFile(File &file);

   void checkpoint();

   ~JournalFile();
 };
}

#endif
