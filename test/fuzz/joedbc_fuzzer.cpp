#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/Readonly_Memory_File.h"
#include "joedb/interpreter/Database.h"
#include "../compiler/testdb.cpp"

/////////////////////////////////////////////////////////////////////////////
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
/////////////////////////////////////////////////////////////////////////////
{
#define CATCH

#ifdef CATCH
 try
 {
#endif
  joedb::Readonly_Memory_File file(Data, Size);
  joedb::Readonly_Journal journal(file, true);
  my_namespace::is_nested::testdb::Database db;
  db.set_max_record_id(Record_Id(journal.get_checkpoint_position()));
  journal.replay_log(db);

#ifdef CATCH
 }
 catch (const joedb::Exception &)
 {
 }
 catch (const joedb::Assertion_Failure &)
 {
 }
#endif

 return 0;
}
