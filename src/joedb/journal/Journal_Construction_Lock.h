#ifndef joedb_Journal_Construction_Lock_declared
#define joedb_Journal_Construction_Lock_declared

#include "joedb/journal/Buffered_File.h"

namespace joedb
{
 /// @ingroup journal
 enum class Construction_Flags
 {
  none = 0,          ///< default
  ignore_header = 1, ///< use file size as checkpoint
  overwrite = 2      ///< allow overwriting an uncheckpointed tail
 };

 /// @ingroup journal
 class Journal_Construction_Lock
 {
  friend class Writable_Journal;

  private:
   bool for_writable_journal = false;
   Journal_Construction_Lock &set_for_writable_journal()
   {
    for_writable_journal = true;
    return *this;
   }

  public:
   Buffered_File &file;
   const Construction_Flags flags;
   const int64_t size;

   bool ignore_header() const
   {
    return int(flags) & int(Construction_Flags::ignore_header);
   };

   bool overwrite() const
   {
    return int(flags) & int(Construction_Flags::overwrite);
   }

   bool is_for_writable_journal() const {return for_writable_journal;}

   explicit Journal_Construction_Lock
   (
    Buffered_File &file,
    Construction_Flags flags = Construction_Flags::none
   );

   Journal_Construction_Lock(const Journal_Construction_Lock &) = delete;
   Journal_Construction_Lock &operator=(const Journal_Construction_Lock &) = delete;

   ~Journal_Construction_Lock();
 };
}

#endif
