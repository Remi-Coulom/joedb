#include "../tutorial.cpp"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/journal/Readonly_Journal.cpp"
#include "joedb/journal/Writable_Journal.cpp"
#include "joedb/journal/Posix_File.cpp"
#include "joedb/Writable.cpp"
#include "joedb/journal/Memory_File.cpp"
#include "joedb/io/type_io.cpp"
#include "joedb/journal/Generic_File.cpp"
#include "joedb/Destructor_Logger.cpp"
#include "external/wide_char_display_width.cpp"

/////////////////////////////////////////////////////////////////////////////
static int local_concurrency(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 tutorial::Local_Client client("local_concurrency.joedb");

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
#if 1
 return joedb::main_exception_catcher(local_concurrency, argc, argv);
#else
 // This does not produce the bug
 return local_concurrency(argc, argv);
#endif
}
