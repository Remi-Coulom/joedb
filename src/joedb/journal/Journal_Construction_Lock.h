#ifndef joedb_Journal_Construction_Lock_declared
#define joedb_Journal_Construction_Lock_declared

#include "joedb/journal/Generic_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Journal_Construction_Lock
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Generic_File &file;
   bool creating_new;

  public:
   explicit Journal_Construction_Lock(Generic_File &file);

   Generic_File &get_file()
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
