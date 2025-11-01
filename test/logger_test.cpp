#include "joedb/error/System_Logger.h"

int main()
{
 joedb::System_Logger logger("joedb_testing");
 logger.log("Hello, log");
 return 0;
}
