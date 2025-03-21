#ifndef JOEDB_ASSERT

#include "joedb/Exception.h"

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

#endif
