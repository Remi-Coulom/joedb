#ifndef joedb_Posix_System_Log_declared
#define joedb_Posix_System_Log_declared

#include "joedb/error/Logger.h"

#include <syslog.h>

namespace joedb
{
 /// @ingroup error
 class Posix_System_Log: public Logger
 {
  private:

  public:
   Posix_System_Log(std::string_view tag);
   void write(std::string_view message) noexcept override;
 };
}

#endif
