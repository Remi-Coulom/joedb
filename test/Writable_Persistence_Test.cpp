#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Writable_Journal.h"

#include "gtest/gtest.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 TEST(Writable_Persistence, basic)
 /////////////////////////////////////////////////////////////////////////////
 {
  Memory_File storage_file;
  Writable_Journal storage(storage_file);

  {
   Memory_File file;
   Writable_Journal journal(file);
   journal.comment("Hello");
   journal.replay_log(storage);
  }

  {
   Memory_File file;
   Writable_Journal journal(file);
   journal.comment("Hella");
   EXPECT_ANY_THROW(journal.replay_log(storage));
  }
 }
}
