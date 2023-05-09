#include <sstream>
#include <string>

#include "joedb/interpreter/Database.h"
#include "joedb/io/Interpreter.h"

/////////////////////////////////////////////////////////////////////////////
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
/////////////////////////////////////////////////////////////////////////////
{
 std::stringstream in(std::string(reinterpret_cast<const char *>(Data), Size));
 std::ostringstream out;

 joedb::Database db(10000);
 joedb::Interpreter interpreter(db, db, nullptr, nullptr, 10000);
 interpreter.main_loop(in, out);

 return 0;
}
