#ifndef joedb_System_Log_declared
#define joedb_System_Log_declared

#ifdef __ANDROID__
#define JOEDB_SYSTEM_LOG Android_System_Log
#elif defined(__APPLE__)
#define JOEDB_SYSTEM_LOG Apple_System_Log
#elif defined(__unix__)
#define JOEDB_SYSTEM_LOG Posix_System_Log
#else
#define JOEDB_SYSTEM_LOG Default_System_Log
#endif

#include "joedb/JOEDB_INCLUDE.h"

#include JOEDB_INCLUDE(JOEDB_SYSTEM_LOG,h)

namespace joedb
{
 using System_Log = JOEDB_SYSTEM_LOG;
}

#endif
