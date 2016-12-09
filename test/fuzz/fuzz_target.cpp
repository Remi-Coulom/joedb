#include <sstream>
#include <string>

#include <joedb/Stream_File.h>
#include <joedb/Journal_File.h>
#include <joedb/Database.h>

#include "DB_Listener.h"
#include "Safe_Listener.h"
#include "Multiplexer.h"
#include "Dummy_Listener.h"

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

  joedb::Safe_Listener safe_listener(db);
  joedb::DB_Listener db_listener(db);
  joedb::Dummy_Listener dummy_listener;

  joedb::Multiplexer multiplexer;
  joedb::Listener &listener = multiplexer.add_listener(dummy_listener);
  multiplexer.add_listener(safe_listener);
  multiplexer.add_listener(db_listener);

  journal.replay_log(listener);
 }

 return 0;
}
