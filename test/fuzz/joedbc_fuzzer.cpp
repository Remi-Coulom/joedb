#include <sstream>
#include <string>

#include "Stream_File.h"
#include "Journal_File.h"
#include "Database.h"
#include "../compiler/testdb.cpp"

/////////////////////////////////////////////////////////////////////////////
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
/////////////////////////////////////////////////////////////////////////////
{
 std::stringstream in(std::string((char *)Data, Size));

 joedb::Stream_File file(in, joedb::Open_Mode::read_existing);
 joedb::Journal_File journal(file);
#if 0
 if (journal.get_state() == joedb::Journal_File::state_t::no_error)
#endif
 {
  testdb::File_Database db("");
  journal.replay_log(db);
 }

 return 0;
}
