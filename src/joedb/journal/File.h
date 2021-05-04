#ifndef joedb_File_declared
#define joedb_File_declared

#ifdef JOEDB_PORTABLE
#define JOEDB_FILE Portable_File
#elif defined(_WIN32) && !defined(__cplusplus_winrt)
#define JOEDB_FILE Windows_File
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#define JOEDB_FILE Posix_File
#else
#error("Error: Unknown system. Suggestion: try defining JOEDB_PORTABLE.")
#endif

#define JOEDB_INCLUDE_STRINGIFY(s) #s
#define JOEDB_INCLUDE_STRINGIFY2(name,extension) JOEDB_INCLUDE_STRINGIFY(name.extension)
#define JOEDB_INCLUDE(name,extension) JOEDB_INCLUDE_STRINGIFY2(name,extension)

#include JOEDB_INCLUDE(JOEDB_FILE,h)

namespace joedb
{
 typedef JOEDB_FILE File;
}

#endif
