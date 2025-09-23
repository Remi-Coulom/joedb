#ifndef joedb_Android_System_Log_declared
#define joedb_Android_System_Log_declared

#include "joedb/error/Logger.h"

#include <string>

namespace joedb
{
 /// @ingroup error
 class Android_System_Log: public Logger
 {
  private:
   std::string tag;

  public:
   Android_System_Log(std::string_view tag);
   void write(std::string_view message) noexcept override;
 };
}

#endif
