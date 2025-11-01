// minimal set of files to be linked with a compiled database
#include "joedb/error/assert.cpp"
#include "joedb/error/Destructor_Logger.cpp"
#include "joedb/Writable.cpp"
#include "joedb/ui/minimal_runtime_io.cpp"
#include "joedb/journal/File.cpp"
#include "joedb/journal/Async_Reader.cpp"
#include "joedb/journal/Abstract_File.cpp"
#include "joedb/journal/File_Buffer.cpp"
#include "joedb/journal/Journal_Construction_Lock.cpp"
#include "joedb/journal/Memory_File.cpp"
#include "joedb/journal/Readonly_Journal.cpp"
#include "joedb/journal/Writable_Journal.cpp"
#include "joedb/error/CLog_Logger.cpp"
#include "joedb/ui/get_time_string.cpp"
#ifdef JOEDB_FILE_IS_PORTABLE_FILE
#include "joedb/journal/Stream_File.cpp"
#endif
