#ifndef JOEDB_ASSERT

#include <stdexcept>

#if defined (NDEBUG) && !defined(JOEDB_FUZZING)
#define JOEDB_ASSERT(x)
#else
#define JOEDB_ASSERT_STRINGIFY(x) #x
#define JOEDB_ASSERT_TO_STRING(x) JOEDB_ASSERT_STRINGIFY(x)
#define JOEDB_ASSERT(x) do\
{\
 if (!(x))\
  throw joedb::Assertion_Failure("!("#x ")\n  File: " __FILE__ "\n  Line: " JOEDB_ASSERT_TO_STRING(__LINE__));\
} while(0)
#endif

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Assertion_Failure: public std::logic_error
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   explicit Assertion_Failure(const char *what_arg):
    std::logic_error(what_arg)
   {
   }
 };
}

#endif
