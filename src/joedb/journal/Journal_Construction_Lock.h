#ifndef joedb_Journal_Construction_Lock_declared
#define joedb_Journal_Construction_Lock_declared

#include "joedb/journal/File_Lock.h"
#include "joedb/journal/Header.h"

namespace joedb
{
 /// @ingroup journal
 enum class Recovery
 {
  none = 0,          ///< default: fail if file size > checkpoint
  ignore_header = 1, ///< use file size as checkpoint
  overwrite = 2      ///< allow overwriting an uncheckpointed tail
 };

 /// @ingroup journal
 class Journal_Construction_Lock
 {
  friend class Writable_Journal;

  private:
   // Acquire the tail before the head, and release them in reverse order.
   File_Tail_Lock tail_lock;
   File_Head_Lock head_lock;

   bool for_writable_journal = false;
   Journal_Construction_Lock &set_for_writable_journal()
   {
    for_writable_journal = true;
    return *this;
   }

  public:
   Abstract_File &file;
   const Recovery recovery;
   const int64_t size;
   Header header;

   bool is_for_writable_journal() const {return for_writable_journal;}

   explicit Journal_Construction_Lock
   (
    Abstract_File &file,
    Recovery recovery = Recovery::none
   );

   Journal_Construction_Lock(const Journal_Construction_Lock &) = delete;
   Journal_Construction_Lock &operator=(const Journal_Construction_Lock &) = delete;
 };
}

#endif
