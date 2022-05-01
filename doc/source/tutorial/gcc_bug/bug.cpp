#include "../tutorial.cpp"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/journal/Readonly_Journal.cpp"
#include "joedb/journal/Writable_Journal.cpp"
#include "joedb/journal/Posix_File.cpp"
#include "joedb/journal/SHA_256.cpp"
#include "joedb/Writable.cpp"
#include "joedb/journal/Memory_File.cpp"
#include "joedb/io/type_io.cpp"
#include "joedb/journal/Generic_File.cpp"
#include "joedb/Destructor_Logger.cpp"
#include "external/wide_char_display_width.cpp"

/////////////////////////////////////////////////////////////////////////////
class Buggy_Client:
/////////////////////////////////////////////////////////////////////////////
 private joedb::Local_Connection<joedb::Posix_File>,
 public tutorial::Client
{
 public:
  Buggy_Client(const char *file_name):
   joedb::Local_Connection<joedb::Posix_File>(file_name),
   tutorial::Client
   (
    *static_cast<joedb::Local_Connection<joedb::Posix_File>*>(this)
   )
  {
  }
};

/////////////////////////////////////////////////////////////////////////////
static int local_concurrency(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
#if 1
 Buggy_Client client("local_concurrency.joedb");
#else
 // This does not produce the bug
 joedb::Local_Connection<joedb::File> connection("local_concurrency.joedb");
 tutorial::Client client(connection);
#endif

 std::cerr << "Yeah, no bug!\n";

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
