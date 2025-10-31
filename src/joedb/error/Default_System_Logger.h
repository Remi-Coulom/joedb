#ifndef joedb_Default_System_Logger_declared
#define joedb_Default_System_Logger_declared

#include "joedb/error/Logger.h"

#include <string>
#include <mutex>

namespace joedb
{
 /// @ingroup error
 class Default_System_Logger: public Logger
 {
  private:
   std::mutex mutex;
   std::string tag;

  public:
   Default_System_Logger(std::string tag);
   void write(const std::string &message) noexcept override;
 };
}

#endif
