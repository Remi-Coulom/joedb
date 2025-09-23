#include "joedb/error/System_Log.h"

int main()
{
 joedb::System_Log logger("joedb_testing");
 logger.write("Hello, log");
 return 0;
}
