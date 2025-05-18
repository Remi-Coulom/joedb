#ifndef JOEDB_DEBUG_ASSERT

#include "joedb/error/Exception.h"

#include <stdexcept>
#include <string>

#define JOEDB_CHECK(x,e) do\
{\
 if (!(x))\
  throw e(joedb::get_error_message(#x, __FILE__, __func__, __LINE__).c_str());\
} while(0)

/// assertion tested in debug mode
/// @ingroup error
#if defined (NDEBUG) && !defined(JOEDB_FUZZING)
#define JOEDB_DEBUG_ASSERT(x)
#else
#define JOEDB_DEBUG_ASSERT(x) JOEDB_CHECK(x, joedb::Assertion_Failure)
#endif

/// always-tested assertion (release and debug mode)
/// @ingroup error
#define JOEDB_RELEASE_ASSERT(x) JOEDB_CHECK(x, joedb::Release_Assertion_Failure)

namespace joedb
{
 std::string get_error_message
 (
  const char *condition,
  const char *file,
  const char *function,
  int line
 );

 using Release_Assertion_Failure = Exception;
 /// Indicates a bug in the code, thrown by @ref JOEDB_DEBUG_ASSERT when NDEBUG not defined
 /// @ingroup error
 class Assertion_Failure: public std::logic_error
 {
  public:
   explicit Assertion_Failure(const char *what_arg):
    std::logic_error(what_arg)
   {
   }
 };
}

#endif
