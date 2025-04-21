#ifndef joedb_Journal_Construction_Lock_declared
#define joedb_Journal_Construction_Lock_declared

#include "joedb/journal/Buffered_File.h"
#include "joedb/journal/Header.h"

namespace joedb
{
 /// @ingroup journal
 class Journal_Construction_Lock
 {
  private:
   Buffered_File &file;
   bool creating_new;

  public:
   Header header;

   explicit Journal_Construction_Lock(Buffered_File &file);

   Journal_Construction_Lock(const Journal_Construction_Lock &) = delete;
   Journal_Construction_Lock &operator=(const Journal_Construction_Lock &) = delete;

   Buffered_File &get_file()
   {
    return file;
   }

   bool is_creating_new() const
   {
    return creating_new;
   }

   ~Journal_Construction_Lock();
 };
}

#endif
