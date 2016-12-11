#include <sstream>
#include <string>

#include "Stream_File.h"
#include "Journal_File.h"
#include "Database.h"
#include "Safe_Listener.h"
#include "DB_Listener.h"

/////////////////////////////////////////////////////////////////////////////
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
/////////////////////////////////////////////////////////////////////////////
{
 std::stringstream in(std::string((char *)Data, Size));

 joedb::Stream_File file(in, joedb::Generic_File::mode_t::read_existing);
 joedb::Journal_File journal(file);
#if 0
 if (journal.get_state() == joedb::Journal_File::state_t::no_error)
#endif
 {
  joedb::Database db;
  joedb::DB_Listener db_listener(db);
  joedb::Safe_Listener safe_listener(db_listener, 1000000);
  journal.replay_log(safe_listener);
 }

 return 0;
}
