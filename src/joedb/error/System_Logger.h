#ifndef joedb_System_Logger_declared
#define joedb_System_Logger_declared

#ifdef __ANDROID__
#include "joedb/error/Android_System_Logger.h"
namespace joedb
{
 using System_Logger = Android_System_Logger;
}
#elif defined(__APPLE__)
#include "joedb/error/Apple_System_Logger.h"
namespace joedb
{
 using System_Logger = Apple_System_Logger;
}
#else
#include "joedb/error/Default_System_Logger.h"
namespace joedb
{
 using System_Logger = Default_System_Logger;
}
#endif

#endif
