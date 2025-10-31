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
   const os_log_t log;

  public:
   Apple_System_Logger(const char *tag);
   Apple_System_Logger(const Apple_System_Logger &) = delete;
   Apple_System_Logger&operator=(const Apple_System_Logger &) = delete;

   void write(const std::string &message) noexcept override;

   ~Apple_System_Logger() override;
 };
}

#endif
