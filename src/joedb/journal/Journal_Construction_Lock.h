#ifndef joedb_Journal_Construction_Lock_declared
#define joedb_Journal_Construction_Lock_declared

#include "joedb/journal/Buffered_File.h"

namespace joedb
{
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

   const enum class Flags
   {
    none = 0,
    ignore_errors = 1,
    overwrite = 2
   } flags;

   const int64_t size;


   bool ignore_errors() const
   {
    return int(flags) & int(Flags::ignore_errors);
   };

   bool overwrite() const
   {
    return int(flags) & int(Flags::overwrite);
   }

   bool is_for_writable_journal() const {return for_writable_journal;}

   explicit Journal_Construction_Lock
   (
    Buffered_File &file,
    Flags flags = Flags::none
   );

   Journal_Construction_Lock(const Journal_Construction_Lock &) = delete;
   Journal_Construction_Lock &operator=(const Journal_Construction_Lock &) = delete;

   ~Journal_Construction_Lock();
 };
}

#endif
