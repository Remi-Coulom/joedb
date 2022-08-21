#include "tutorial.h"
#include "joedb/io/main_exception_catcher.h"

#include <chrono>
#include <thread>

/////////////////////////////////////////////////////////////////////////////
static int local_concurrency(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
#ifdef JOEDB_FILE_IS_LOCKABLE
 tutorial::Local_Client client("local_concurrency.joedb");

 while (true)
 {
  client.transaction([](tutorial::Generic_File_Database &db)
  {
   db.new_person();
  });

  std::cout << "I have just added one person. Total number of persons: ";
  std::cout << client.get_database().get_person_table().get_size() << '\n';
  std::this_thread::sleep_for(std::chrono::seconds(1));
 }
#else
 std::cout << "File locking is not supported on this platform.\n";
#endif

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(local_concurrency, argc, argv);
}
