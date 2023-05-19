#include "tutorial.h"

#include "joedb/concurrency/Embedded_Connection.h"
#include "joedb/journal/Memory_Journal.h"

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 //
 // This sets up a configuration with a server and 2 clients.
 //
 joedb::Memory_Journal server_journal;
 joedb::Memory_Journal client1_journal;
 joedb::Memory_Journal client2_journal;

 joedb::Embedded_Connection connection1(client1_journal, server_journal);
 joedb::Embedded_Connection connection2(client2_journal, server_journal);

 tutorial::Client client1(connection1);
 tutorial::Client client2(connection2);

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
 client1.transaction([](tutorial::Journal_Database &db)
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
