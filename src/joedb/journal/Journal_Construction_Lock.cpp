#include "joedb/journal/Journal_Construction_Lock.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Journal_Construction_Lock::Journal_Construction_Lock
 ////////////////////////////////////////////////////////////////////////////
 (
  Buffered_File &file,
  bool ignore_errors
 ):
  file(file),
  ignore_errors(ignore_errors),
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
