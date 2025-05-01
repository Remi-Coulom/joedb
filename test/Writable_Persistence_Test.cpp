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
   journal.soft_checkpoint();
   EXPECT_EQ(storage.get_checkpoint(), 41);
   journal.replay_log(storage);
   EXPECT_EQ(storage.get_checkpoint(), 48);
  }

  {
   Memory_File file;
   Writable_Journal journal(file);
   journal.comment("Hella");
   journal.soft_checkpoint();
   EXPECT_ANY_THROW(journal.start_writing(41));
  }
 }
}
