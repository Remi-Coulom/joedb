#include "tutorial.h"

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 //
 // This sets up a configuration with 2 client accessing the same file
 //
 const char * const file_name = "concurrency.joedb";
 std::remove(file_name);
 tutorial::Local_Client client1(file_name);
 tutorial::Local_Client client2(file_name);

 //
 // The databases are empty. client1 will add a few cities.
 //
 // All write operations are performed via the transaction function.
 // The transaction function takes a lambda as parameter.
 // The lock_pull operation is performed before the lambda, and the push_unlock
 // operation is performed after the lambda, if no exception was thrown.
 // If any exception was thrown during the lambda, then the changes
 // are not pushed to the server, and the server is unlocked.
 // Writes that occured in a transaction before an exception are not sent to
 // the server, but they are written locally.
 //
 client1.transaction([](tutorial::Generic_File_Database &db)
 {
  db.new_city("Paris");
  db.new_city("New York");
  db.new_city("Tokyo");
 });

 //
 // client1.get_database() gives a read-only access to the local copy
 //
 std::cout << "Number of cities for client1: ";
 std::cout << client1.get_database().get_city_table().get_size() << '\n';

 //
 // Client1 added cities, and they were pushed to the central database.
 // They have not yet reached client2.
 //
 std::cout << "Number of cities for client2 before pulling: ";
 std::cout << client2.get_database().get_city_table().get_size() << '\n';

 //
 // Let's pull to update the database of client2
 //
 client2.pull();
 std::cout << "Number of cities for client2 after pulling: ";
 std::cout << client2.get_database().get_city_table().get_size() << '\n';

 return 0;
}
