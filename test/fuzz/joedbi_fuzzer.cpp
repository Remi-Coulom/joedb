#include <sstream>
#include <string>

#include "joedb/interpreted/Database.h"
#include "joedb/ui/Interpreter.h"

/////////////////////////////////////////////////////////////////////////////
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
/////////////////////////////////////////////////////////////////////////////
{
 std::istringstream in(std::string(reinterpret_cast<const char *>(Data), Size));
 std::ostringstream out;

 const joedb::Record_Id max_record_id{10000};
 joedb::Database db(max_record_id);
 joedb::Interpreter interpreter(db, db, max_record_id);

 try
 {
  interpreter.main_loop(in, out);
 }
 catch (const joedb::Exception &)
 {
 }

 return 0;
}
