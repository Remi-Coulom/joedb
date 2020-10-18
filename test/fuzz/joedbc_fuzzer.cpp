#include <sstream>
#include <string>

#include "Stream_File.h"
#include "Readonly_Journal.h"
#include "Database.h"
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

  std::stringstream in(std::string((char *)Data, Size));
  joedb::Stream_File file(in, joedb::Open_Mode::read_existing);
  joedb::Readonly_Journal journal(file, true);
  testdb::Database db;
  db.set_max_record_id(journal.get_checkpoint_position());
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
