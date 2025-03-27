#ifndef joedb_error_Exception_declared
#define joedb_error_Exception_declared

#include <stdexcept>

namespace joedb::error
{
 ////////////////////////////////////////////////////////////////////////////
 class Exception: public std::runtime_error
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   explicit Exception(const char *what_arg):
    std::runtime_error(what_arg)
   {
   }

   explicit Exception(const std::string &what_arg):
    std::runtime_error(what_arg)
   {
   }
 };
}

namespace joedb
{
 using Exception = error::Exception;
}

#endif
