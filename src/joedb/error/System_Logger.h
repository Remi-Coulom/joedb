#ifndef joedb_System_Logger_declared
#define joedb_System_Logger_declared

#ifdef __ANDROID__
#define JOEDB_SYSTEM_LOG Android_System_Logger
#elif defined(__APPLE__)
#define JOEDB_SYSTEM_LOG Apple_System_Logger
#elif defined(__unix__)
#define JOEDB_SYSTEM_LOG Posix_System_Logger
#else
#define JOEDB_SYSTEM_LOG Default_System_Logger
#endif

#include "joedb/JOEDB_INCLUDE.h"

#include JOEDB_INCLUDE(JOEDB_SYSTEM_LOG,h)

namespace joedb
{
 using System_Logger = JOEDB_SYSTEM_LOG;
}

#endif
