#include <sstream>
#include <string>

#include "Database.h"
#include "Interpreter.h"

/////////////////////////////////////////////////////////////////////////////
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
/////////////////////////////////////////////////////////////////////////////
{
 std::stringstream in(std::string((char *)Data, Size));
 std::ostringstream out;

 joedb::Database db;
 joedb::Interpreter interpreter(db);
 interpreter.main_loop(in, out);

 return 0;
}
