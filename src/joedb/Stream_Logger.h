#ifndef joedb_Stream_Logger
#define joedb_Stream_Logger

#include "joedb/Logger.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Stream_Logger: public Logger
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::ostream &out;

   void write(const char *message) noexcept override
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
