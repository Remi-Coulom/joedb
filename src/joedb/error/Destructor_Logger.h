#ifndef joedb_Destructor_Logger
#define joedb_Destructor_Logger

#include <string>
#include <mutex>

namespace joedb
{
 class Logger;

 /// @ingroup error
 class Destructor_Logger
 {
  private:
   static Logger *the_logger;
   static std::mutex mutex;

  public:
   static void warning(const std::string &message) noexcept;
   static void set_logger(Logger *new_logger);
   static void remove_logger();
   static void set_logger();
 };
}

#endif
