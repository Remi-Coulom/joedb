#ifndef joedb_Posix_System_Logger_declared
#define joedb_Posix_System_Logger_declared

#include "joedb/error/Logger.h"

#include <syslog.h>

namespace joedb
{
 /// @ingroup error
 class Posix_System_Logger: public Logger
 {
  private:

  public:
   Posix_System_Logger(std::string_view tag);
   void write(std::string_view message) noexcept override;
 };
}

#endif
