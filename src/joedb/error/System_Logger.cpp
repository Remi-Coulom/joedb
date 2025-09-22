#ifdef __ANDROID__
#include "joedb/error/Android_System_Logger.cpp"
#elif defined(__APPLE__)
#include "joedb/error/Apple_System_Logger.cpp"
#else
#include "joedb/error/Default_System_Logger.cpp"
#endif
