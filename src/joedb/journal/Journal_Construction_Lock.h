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
   explicit Journal_Construction_Lock(Generic_File &file): file(file)
   {
    if (file.is_shared())
     file.exclusive_lock();
    else if (file.get_mode() == Open_Mode::read_existing)
     file.shared_lock();

    creating_new = file.get_mode() != Open_Mode::read_existing &&
     file.get_size() == 0;
   }

   Generic_File &get_file()
   {
    return file;
   }

   bool is_creating_new() const
   {
    return creating_new;
   }

   ~Journal_Construction_Lock()
   {
    try
    {
     if (file.is_shared() || file.get_mode() == Open_Mode::read_existing)
      file.unlock();
    }
    catch(...)
    {
    }
   }
 };
}

#endif
