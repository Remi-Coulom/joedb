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
   JournalFile(const char *file_name, bool read_only);

   bool is_good() const {return file.is_good();}
   bool is_read_only() const {return file.is_read_only();}

   void checkpoint();

   ~JournalFile();
 };
}

#endif
