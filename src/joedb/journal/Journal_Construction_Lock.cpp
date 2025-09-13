#include "joedb/journal/Journal_Construction_Lock.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Journal_Construction_Lock::Journal_Construction_Lock
 ////////////////////////////////////////////////////////////////////////////
 (
  Abstract_File &file,
  Recovery recovery
 ):
  file(file),
  recovery(recovery),
  size(file.get_size())
 {
  if (file.is_readonly())
   file.shared_lock_head();
  else
  {
   if (file.is_shared())
    file.exclusive_lock_tail();
   file.exclusive_lock_head();
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
