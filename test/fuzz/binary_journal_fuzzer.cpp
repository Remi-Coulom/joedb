#include <sstream>
#include <string>

#include "Stream_File.h"
#include "Journal_File.h"
#include "Database.h"
#include "Safe_Writeable.h"
#include "DB_Writeable.h"

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
  joedb::Safe_Writeable safe_writeable(1000000);
  journal.replay_log(safe_writeable);
 }

 return 0;
}
