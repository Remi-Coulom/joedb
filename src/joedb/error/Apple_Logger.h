#ifndef joedb_Apple_Logger_declared
#define joedb_Apple_Logger_declared

#include "joedb/error/Logger.h"

#include <os/log.h>

namespace joedb
{
 /// @ingroup error
 class Apple_Logger: public Logger
 {
  private:
   const os_log_t os_log;

  public:
   Apple_Logger(const char *tag);
   Apple_Logger(const Apple_Logger &) = delete;
   Apple_Logger&operator=(const Apple_Logger &) = delete;

   void log(const std::string &message) noexcept override;

   ~Apple_Logger() override;
 };
}

#endif
