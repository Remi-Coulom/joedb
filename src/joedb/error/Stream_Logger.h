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

   void write(const std::string &message) noexcept override
   {
    try
    {
     std::lock_guard lock(mutex);
     out << message << '\n';
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
