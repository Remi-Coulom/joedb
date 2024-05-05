// minimal set of files to be linked with a compiled database
#include "joedb/Destructor_Logger.cpp"
#include "joedb/Posthumous_Catcher.cpp"
#include "joedb/Posthumous_Thrower.cpp"
#include "joedb/Writable.cpp"
#include "joedb/compiler/minimal_runtime_io.cpp"
#include "joedb/journal/Abstract_File.cpp"
#include "joedb/journal/File.cpp"
#include "joedb/journal/Generic_File.cpp"
#include "joedb/journal/Journal_Construction_Lock.cpp"
#include "joedb/journal/Memory_File.cpp"
#include "joedb/journal/Readonly_Journal.cpp"
#include "joedb/journal/Writable_Journal.cpp"
#include "joedb/journal/SHA_256.cpp"
#include "joedb/Blob.cpp"
#ifdef JOEDB_FILE_IS_PORTABLE_FILE
#include "joedb/journal/Stream_File.cpp"
#endif
