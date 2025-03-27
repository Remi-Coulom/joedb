#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/Readonly_Memory_File.h"
#include "../compiler/db/test/writable.cpp"

/////////////////////////////////////////////////////////////////////////////
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
/////////////////////////////////////////////////////////////////////////////
{
 try
 {
  joedb::Readonly_Memory_File file(Data, Size);
  joedb::Readonly_Journal journal(file, joedb::Readonly_Journal::Check::none);
  my_namespace::is_nested::test::Database db;
  db.set_max_record_id(size_t(journal.get_checkpoint_position()));
  journal.replay_log(db);
 }
 catch (const joedb::error::Exception &)
 {
 }

 return 0;
}
