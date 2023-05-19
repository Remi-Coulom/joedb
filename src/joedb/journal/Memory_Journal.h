#ifndef joedb_Memory_Journal_declared
#define joedb_Memory_Journal_declared

#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/Memory_File.h"

namespace joedb
{
 class Memory_Journal: public Memory_File, public Writable_Journal
 {
  public:
   Memory_Journal(): Writable_Journal(*static_cast<Memory_File *>(this))
   {
   }
 };
}

#endif
