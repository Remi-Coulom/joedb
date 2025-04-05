#ifndef JOEDB_ASSERT

#include <stdexcept>

#define JOEDB_ASSERT_STRINGIFY(x) #x
#define JOEDB_ASSERT_TO_STRING(x) JOEDB_ASSERT_STRINGIFY(x)
#define JOEDB_CHECK(x,e) do\
{\
 if (!(x))\
  throw e("!("#x ")\n  File: " __FILE__ "\n  Line: " JOEDB_ASSERT_TO_STRING(__LINE__));\
} while(0)

#if defined (NDEBUG) && !defined(JOEDB_FUZZING)
/// @ingroup error
#define JOEDB_ASSERT(x)
#else
/// @ingroup error
#define JOEDB_ASSERT(x) JOEDB_CHECK(x, joedb::Assertion_Failure)
#endif

/// @ingroup error
#define JOEDB_RELEASE_ASSERT(x) JOEDB_CHECK(x, joedb::Exception)

namespace joedb
{
 /// Indicates a bug in the code, thrown by @ref JOEDB_ASSERT when NDEBUG not defined
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
