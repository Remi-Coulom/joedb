#ifndef joedb_Android_System_Logger_declared
#define joedb_Android_System_Logger_declared

#include "joedb/error/Logger.h"
#include <android/log.h>

namespace joedb
{
 /// @ingroup error
 class Android_System_Logger: public Logger
 {
  private:
  public:
   Android_System_Logger(const std::string &logger_id)
   {
   }

   void write(std::string_view message) noexcept override
   {
   }
 };
}

#endif
