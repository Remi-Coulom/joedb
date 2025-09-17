#ifndef joedb_System_Logger_declared
#define joedb_System_Logger_declared

#ifdef __ANDROID__
#include "joedb/error/Android_System_Logger.h"
namespace joedb
{
 using System_Logger = Android_System_Logger;
}
#elif defined(APPLE)
#else
#endif

#endif
