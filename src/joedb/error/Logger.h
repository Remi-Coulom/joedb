#ifndef joedb_Logger_declared
#define joedb_Logger_declared

#include <string_view>

namespace joedb
{
 /// @ingroup error
 class Logger
 {
  public:
   virtual void write(std::string_view message) noexcept {}
   virtual ~Logger() = default;
 };
}

#endif
