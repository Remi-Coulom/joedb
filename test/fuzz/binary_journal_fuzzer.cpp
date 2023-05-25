#include "joedb/journal/Readonly_Memory_File.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/interpreter/Database.h"

/////////////////////////////////////////////////////////////////////////////
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
/////////////////////////////////////////////////////////////////////////////
{
 try
 {
  joedb::Readonly_Memory_File file(Data, Size);
  joedb::Readonly_Journal journal(file, joedb::Readonly_Journal::Check::none);
  joedb::Database db(1000000);
  journal.replay_log(db);
 }
 catch (const joedb::Exception &)
 {
 }
 catch (const joedb::Assertion_Failure &)
 {
 }

 return 0;
}
