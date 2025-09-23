#ifndef joedb_Apple_System_Logger_declared
#define joedb_Apple_System_Logger_declared

#include "joedb/error/Logger.h"

#include <os/log.h>

namespace joedb
{
 /// @ingroup error
 class Apple_System_Logger: public Logger
 {
  private:
   os_log_t log;

  public:
   Apple_System_Logger(std::string_view tag);
   void write(std::string_view message) noexcept override;
 };
}

#endif
