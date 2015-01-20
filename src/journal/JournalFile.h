#ifndef joedb_JournalFile_declared
#define joedb_JournalFile_declared

#include "File.h"

namespace joedb
{
 class JournalFile
 {
  private:
   File file;

   int checkpoint_index;

  public:
   JournalFile(const char *file_name, File::open_mode_t open_mode);

   bool is_good() const {return file.is_good();}

   void checkpoint();

   ~JournalFile();
 };
}

#endif
