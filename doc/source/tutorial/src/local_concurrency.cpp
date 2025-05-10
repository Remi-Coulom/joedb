#include "tutorial/File_Client.h"

#include "joedb/ui/main_wrapper.h"

#include <chrono>
#include <thread>

/////////////////////////////////////////////////////////////////////////////
static int local_concurrency(joedb::Arguments &arguments)
/////////////////////////////////////////////////////////////////////////////
{
 tutorial::File_Client client("local_concurrency.joedb");

 while (true)
 {
  client.transaction([](tutorial::Writable_Database &db)
  {
   db.new_person();
  });

  std::cout << "I have just added one person. Population: ";
  std::cout << client.get_database().get_person_table().get_size() << '\n';
  std::this_thread::sleep_for(std::chrono::seconds(1));
 }

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_wrapper(local_concurrency, argc, argv);
}
