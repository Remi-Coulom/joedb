#include "joedb/ui/main_exception_catcher.h"
#include "joedb/error/Posthumous_Catcher.h"
#include "tutorial/File_Client.h"

static int client_lock(int argc, char **argv)
{
 tutorial::File_Client client("tutorial.joedb");
 joedb::Posthumous_Catcher catcher;

 {
  tutorial::Client_Lock lock(client);
  lock.set_catcher(catcher);

  lock.get_database().write_comment("Hello");
  lock.push();
  lock.get_database().write_comment("Goodbye");
  lock.get_database().write_timestamp();
 }

 catcher.rethrow();

 return 0;
}

int main(int argc, char **argv)
{
 return joedb::main_exception_catcher(client_lock, argc, argv);
}
