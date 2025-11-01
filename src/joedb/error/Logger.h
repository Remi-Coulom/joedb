#ifndef joedb_Logger_declared
#define joedb_Logger_declared

#include <string>

namespace joedb
{
 /// @ingroup error
 class Logger
 {
  public:
   virtual void log(const std::string &message) noexcept {}
   virtual ~Logger() = default;
   static Logger dummy_logger;
 };
}

#endif
