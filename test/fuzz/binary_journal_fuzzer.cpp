#include <sstream>
#include <string>

#include "joedb/journal/Stream_File.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/interpreter/Database.h"

/////////////////////////////////////////////////////////////////////////////
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
/////////////////////////////////////////////////////////////////////////////
{
 try
 {
  std::stringstream in(std::string((char *)Data, Size));
  joedb::Stream_File file(in, joedb::Open_Mode::read_existing);
  joedb::Readonly_Journal journal(file, true);
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
