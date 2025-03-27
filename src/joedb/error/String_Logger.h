#ifndef joedb_String_Logger
#define joedb_String_Logger

#include "joedb/error/Logger.h"

#include <string>

namespace joedb::error
{
 ////////////////////////////////////////////////////////////////////////////
 class String_Logger: public Logger
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::string message_memory;

  public:
   void write(const char *message) noexcept final override
   {
    if (message)
     message_memory = message;
    else
     message_memory.clear();
   }

   const std::string &get_message() const
   {
    return message_memory;
   }

   static String_Logger the_logger;
 };
}

#endif
