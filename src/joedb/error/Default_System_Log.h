#ifndef joedb_Default_System_Log_declared
#define joedb_Default_System_Log_declared

#include "joedb/error/Logger.h"

#include <string>
#include <mutex>

namespace joedb
{
 /// @ingroup error
 class Default_System_Log: public Logger
 {
  private:
   std::mutex mutex;
   std::string tag;

  public:
   Default_System_Log(std::string_view tag);
   void write(std::string_view message) noexcept override;
 };
}

#endif
