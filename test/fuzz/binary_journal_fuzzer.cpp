#include "joedb/journal/Readonly_Memory_File.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/interpreted/Database.h"

/////////////////////////////////////////////////////////////////////////////
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
/////////////////////////////////////////////////////////////////////////////
{
 try
 {
  joedb::Readonly_Memory_File file(Data, Size);
  joedb::Readonly_Journal journal(joedb::Journal_Construction_Lock(file, true));
  joedb::Database db(1000000);
  journal.replay_log(db);
 }
 catch (const joedb::Exception &)
 {
 }
 catch (const std::bad_variant_access &)
 {
 }

 return 0;
}
