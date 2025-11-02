#ifndef joedb_System_Logger_declared
#define joedb_System_Logger_declared

#ifdef __ANDROID__
#define JOEDB_SYSTEM_LOGGER Android_Logger
#elif defined(__APPLE__)
#define JOEDB_SYSTEM_LOGGER Apple_Logger
#elif defined(__unix__)
#define JOEDB_SYSTEM_LOGGER Posix_Logger
#else
#define JOEDB_SYSTEM_LOGGER CLog_Logger
#define JOEDB_SYSTEM_LOGGER_NO_CPP
#endif

#include "joedb/JOEDB_INCLUDE.h"

#include JOEDB_INCLUDE(JOEDB_SYSTEM_LOGGER,h)

namespace joedb
{
 using System_Logger = JOEDB_SYSTEM_LOGGER;
}

#endif
