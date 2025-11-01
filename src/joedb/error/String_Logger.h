#ifndef joedb_String_Logger
#define joedb_String_Logger

#include "joedb/error/Logger.h"

#include <string>

namespace joedb
{
 /// @ingroup error
 class String_Logger: public Logger
 {
  private:
   std::string message_memory;

  public:
   void log(const std::string &message) noexcept override
   {
    message_memory = message;
   }

   const std::string &get_message() const
   {
    return message_memory;
   }

   static String_Logger the_logger;
 };
}

#endif
