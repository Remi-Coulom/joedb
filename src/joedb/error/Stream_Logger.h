#ifndef joedb_error_Stream_Logger
#define joedb_error_Stream_Logger

#include "joedb/error/Logger.h"

#include <iostream>

namespace joedb::error
{
 ////////////////////////////////////////////////////////////////////////////
 class Stream_Logger: public Logger
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::ostream &out;

   void write(const char *message) noexcept final
   {
    try
    {
     out << "joedb: " << message << '\n';
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
