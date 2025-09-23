#include "joedb/error/System_Logger.h"

int main()
{
 joedb::System_Logger logger("joedb_testing");
 logger.write("Hello, log");
 return 0;
}
