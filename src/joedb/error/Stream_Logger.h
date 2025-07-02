#ifndef joedb_Stream_Logger
#define joedb_Stream_Logger

#include "joedb/error/Logger.h"

#include <ostream>
#include <mutex>

namespace joedb
{
 /// @ingroup error
 class Stream_Logger: public Logger
 {
  private:
   std::ostream &out;
   std::mutex mutex;

   void write(std::string_view message) noexcept override
   {
    try
    {
     std::lock_guard lock(mutex);
     out << message;
    }
    catch (...)
    {
    }
   }

  public:
   Stream_Logger(std::ostream &out): out(out) {}
 };
}

#endif
