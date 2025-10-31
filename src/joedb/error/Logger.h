#ifndef joedb_Logger_declared
#define joedb_Logger_declared

#include <string>

namespace joedb
{
 /// @ingroup error
 class Logger
 {
  public:
   virtual void write(const std::string &message) noexcept {}
   virtual ~Logger() = default;
 };
}

#endif
