#ifndef joedb_Exception_declared
#define joedb_Exception_declared

#include <stdexcept>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Runtime_Error: public std::runtime_error
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   explicit Runtime_Error(const char *what_arg):
    std::runtime_error(what_arg)
   {
   }

   explicit Runtime_Error(const std::string &what_arg):
    std::runtime_error(what_arg)
   {
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Assertion_Failure: public Runtime_Error
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   explicit Assertion_Failure(const char *what_arg):
    Runtime_Error(what_arg)
   {
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Exception: public Runtime_Error
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   explicit Exception(const char *what_arg):
    Runtime_Error(what_arg)
   {
   }

   explicit Exception(const std::string &what_arg):
    Runtime_Error(what_arg)
   {
   }
 };
}

#endif
