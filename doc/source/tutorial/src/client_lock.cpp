#include "joedb/ui/main_exception_catcher.h"
#include "tutorial/File_Client.h"

static int client_lock(int argc, char **argv)
{
 tutorial::File_Client client("tutorial.joedb");

 {
  tutorial::Client_Lock lock(client);

  lock.get_database().write_comment("Hello");
  lock.push();
  lock.get_database().write_comment("Goodbye");
  lock.get_database().write_timestamp();
  lock.push_unlock();
 }

 return 0;
}

int main(int argc, char **argv)
{
 return joedb::main_exception_catcher(client_lock, argc, argv);
}
