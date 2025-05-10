#include "joedb/ui/main_wrapper.h"
#include "tutorial/File_Client.h"

static int client_lock(joedb::Arguments &arguments)
{
 tutorial::File_Client client("tutorial.joedb");

 {
  tutorial::Client_Lock lock(client);

  lock.get_database().write_comment("Hello");
  lock.checkpoint_and_push();
  lock.get_database().write_comment("Goodbye");
  lock.get_database().write_timestamp();
  lock.checkpoint_and_push_unlock();
 }

 return 0;
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(client_lock, argc, argv);
}
