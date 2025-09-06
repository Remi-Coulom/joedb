#ifndef joedb_Logger
#define joedb_Logger

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
