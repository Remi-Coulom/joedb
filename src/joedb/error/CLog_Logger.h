#ifndef joedb_CLog_Logger_declared
#define joedb_CLog_Logger_declared

#include "joedb/error/Stream_Logger.h"

#include <iostream>

namespace joedb
{
 /// @ingroup error
 class CLog_Logger: public Stream_Logger
 {
  public:
   CLog_Logger(std::string tag = ""): Stream_Logger(std::clog, tag) {}
 };
}

#endif
