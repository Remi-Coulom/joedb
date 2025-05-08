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
  joedb::Readonly_Journal journal(joedb::Journal_Construction_Lock(file, true));
  my_namespace::is_nested::test::Database db;
  db.set_max_record_id(journal.get_checkpoint());
  journal.replay_log(db);
 }
 catch (const joedb::Exception &)
 {
 }

 return 0;
}
