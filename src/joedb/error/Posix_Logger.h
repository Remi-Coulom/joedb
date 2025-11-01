#ifndef joedb_Posix_Logger_declared
#define joedb_Posix_Logger_declared

#include "joedb/error/Logger.h"

namespace joedb
{
 /// @ingroup error
 class Posix_Logger: public Logger
 {
  private:
   const std::string tag;

  public:
   Posix_Logger(std::string tag);
   void write(const std::string &message) noexcept override;
 };
}

#endif
