#include <sstream>
#include <string>

#include <joedb/Stream_File.h>
#include <joedb/Journal_File.h>
#include <joedb/Database.h>

#include "DB_Listener.h"

/////////////////////////////////////////////////////////////////////////////
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
/////////////////////////////////////////////////////////////////////////////
{
 std::stringstream in(std::string((char *)Data, Size));

 joedb::Stream_File file(in, joedb::Generic_File::mode_t::read_existing);
 joedb::Journal_File journal(file);
 if (journal.get_state() == joedb::Journal_File::state_t::no_error)
 {
  joedb::Database db;
  joedb::DB_Listener listener(db);
  journal.replay_log(listener);
 }

 return 0;
}
