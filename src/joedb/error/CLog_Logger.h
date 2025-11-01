#ifndef joedb_CLog_Logger_declared
#define joedb_CLog_Logger_declared

#include "joedb/error/Logger.h"

#include <string>
#include <mutex>

namespace joedb
{
 /// @ingroup error
 class CLog_Logger: public Logger
 {
  private:
   const std::string tag;
   static std::mutex mutex;

  public:
   CLog_Logger(std::string tag = "");
   void write(const std::string &message) noexcept override;
 };
}

#endif
