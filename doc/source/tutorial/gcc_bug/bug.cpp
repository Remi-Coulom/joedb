#include "joedb/Destructor_Logger.cpp"
#include "joedb/Writable.cpp"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/journal/Generic_File.cpp"
#include "joedb/journal/Memory_File.cpp"
#include "joedb/journal/Posix_File.cpp"
#include "joedb/journal/Readonly_Journal.cpp"
#include "joedb/journal/SHA_256.cpp"
#include "joedb/journal/Writable_Journal.cpp"
#include "joedb/concurrency/Client.h"
#include "../../../../test/compiler/db/empty.cpp"

/////////////////////////////////////////////////////////////////////////////
class Buggy_Client:
/////////////////////////////////////////////////////////////////////////////
 private joedb::Local_Connection<joedb::Posix_File>,
 public empty::Client
{
 public:
  Buggy_Client(const char *file_name):
   joedb::Local_Connection<joedb::Posix_File>(file_name),
   empty::Client
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
 empty::Client client(connection);
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
