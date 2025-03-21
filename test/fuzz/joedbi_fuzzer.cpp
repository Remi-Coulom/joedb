#include <sstream>
#include <string>

#include "joedb/interpreter/Database.h"
#include "joedb/io/Interpreter.h"

/////////////////////////////////////////////////////////////////////////////
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
/////////////////////////////////////////////////////////////////////////////
{
 std::istringstream in(std::string(reinterpret_cast<const char *>(Data), Size));
 std::ostringstream out;

 joedb::Database db(10000);
 joedb::Interpreter interpreter(db, db, nullptr, db, 10000);

 try
 {
  interpreter.main_loop(in, out);
 }
 catch (const joedb::Exception &)
 {
 }

 return 0;
}
