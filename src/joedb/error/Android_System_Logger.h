#ifndef joedb_Android_System_Logger_declared
#define joedb_Android_System_Logger_declared

#include "joedb/error/Logger.h"

#include <string>

namespace joedb
{
 /// @ingroup error
 class Android_System_Logger: public Logger
 {
  private:
   std::string tag;

  public:
   Android_System_Logger(std::string_view tag);
   void write(std::string_view message) noexcept override;
 };
}

#endif
