#ifndef joedb_File_Lock_declared
#define joedb_File_Lock_declared

#include "joedb/journal/Abstract_File.h"

namespace joedb
{
 class File_Head_Lock
 {
  public:
   enum class Mode {shared, exclusive};

  private:
   Abstract_File &file;

  public:
   File_Head_Lock(Abstract_File &file, Mode mode): file(file)
   {
    if (mode == Mode::shared)
     file.shared_lock_head();
    else
     file.exclusive_lock_head();
   }

   File_Head_Lock(const File_Head_Lock &) = delete;
   File_Head_Lock &operator=(const File_Head_Lock &) = delete;

   ~File_Head_Lock()
   {
    file.unlock_head();
   }
 };

 class File_Tail_Lock
 {
  private:
   Abstract_File *file;

  public:
   explicit File_Tail_Lock(Abstract_File *file): file(file)
   {
    if (file)
     file->exclusive_lock_tail();
   }

   File_Tail_Lock(const File_Tail_Lock &) = delete;
   File_Tail_Lock &operator=(const File_Tail_Lock &) = delete;

   /// Leave the file locked, transferring unlock responsibility to the caller.
   void release() noexcept {file = nullptr;}

   ~File_Tail_Lock()
   {
    if (file)
     file->unlock_tail();
   }
 };
}

#endif
