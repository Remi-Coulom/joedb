#ifndef joedb_File_declared
#define joedb_File_declared

#ifdef JOEDB_PORTABLE
#define JOEDB_FILE Portable_File
#define JOEDB_FILE_IS_PORTABLE_FILE
#elif defined(_WIN32) && !defined(__cplusplus_winrt)
#define JOEDB_FILE Windows_File
#define JOEDB_FILE_IS_WINDOWS_FILE
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#define JOEDB_FILE Posix_File
#define JOEDB_FILE_IS_POSIX_FILE
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
