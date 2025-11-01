#ifndef joedb_Android_Logger_declared
#define joedb_Android_Logger_declared

#include "joedb/error/Logger.h"

namespace joedb
{
 /// @ingroup error
 class Android_Logger: public Logger
 {
  private:
   const std::string tag;

  public:
   Android_Logger(std::string tag);
   void log(const std::string &message) noexcept override;
 };
}

#endif
