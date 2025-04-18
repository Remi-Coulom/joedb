#include "joedb/journal/Journal_Construction_Lock.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Journal_Construction_Lock::Journal_Construction_Lock
 ////////////////////////////////////////////////////////////////////////////
 (
  Buffered_File &file
 ):
  file(file)
 {
  if (file.is_readonly())
  {
   file.shared_lock_head();
   creating_new = false;
  }
  else
  {
   if (file.is_shared())
    file.exclusive_lock_tail();
   file.exclusive_lock_head();
   creating_new = file.get_size() == 0;
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 Journal_Construction_Lock::~Journal_Construction_Lock()
 ////////////////////////////////////////////////////////////////////////////
 {
  file.unlock_head();
  if (file.is_shared())
   file.unlock_tail();
 }
}
