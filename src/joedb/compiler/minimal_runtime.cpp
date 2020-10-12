// minimal set of files to be linked with a compiled database
#include "joedb/Writable.cpp"
#include "joedb/compiler/minimal_runtime_io.cpp"
#include "joedb/journal/File.cpp"
#include "joedb/journal/Generic_File.cpp"
#include "joedb/journal/Journal_File.cpp"
#include "joedb/journal/Readonly_Journal.cpp"
#include "joedb/journal/Stream_File.cpp"
