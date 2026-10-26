#ifndef joedb_Global_Logger
#define joedb_Global_Logger

#include "external/cstring_view.hpp"
#include <mutex>

namespace joedb
{
 class Logger;

 /// @ingroup error
 class Global_Logger
 {
  private:
   static Logger *the_logger;
   static std::mutex mutex;

  public:
   static void warning(beman::cstring_view message) noexcept;
   static void set_logger(Logger *new_logger);
   static void remove_logger();
   static void set_logger();
 };
}

#endif
