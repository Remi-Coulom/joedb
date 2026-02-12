#ifndef joedb_Stream_Logger_declared
#define joedb_Stream_Logger_declared

#include "joedb/error/Logger.h"

#include <iosfwd>
#include <mutex>

namespace joedb
{
 /// @ingroup error
 class Stream_Logger: public Logger
 {
  private:
   std::ostream &out;
   const std::string tag;
   std::mutex mutex;

  public:
   Stream_Logger(std::ostream &out, std::string tag = "");
   void log(const std::string &message) noexcept override;
 };
}

#endif
