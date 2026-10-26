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
  tail_lock(file.is_shared() ? &file : nullptr),
  head_lock
  (
   file,
   file.is_readonly() ? File_Head_Lock::Mode::shared : File_Head_Lock::Mode::exclusive
  ),
  file(file),
  recovery(recovery),
  size(file.get_size())
 {
 }
}
