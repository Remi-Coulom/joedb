#ifndef joedb_Destructor_Logger
#define joedb_Destructor_Logger

#include <string_view>

namespace joedb
{
 class Logger;

 /// @ingroup error
 class Destructor_Logger
 {
  private:
   static Logger *the_logger;

  public:
   static void warning(std::string_view message) noexcept;
   static void set_logger(Logger *new_logger);
   static void remove_logger();
   static void set_logger();
 };
}

#endif
