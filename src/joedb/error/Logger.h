#ifndef joedb_Logger_declared
#define joedb_Logger_declared

#include "external/cstring_view.hpp"

namespace joedb
{
 /// @ingroup error
 class Logger
 {
  public:
   virtual void log(beman::cstring_view message) noexcept {}
   virtual ~Logger() = default;
   static Logger dummy_logger;
 };
}

#endif
