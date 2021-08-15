#include "tutorial.h"

#include <joedb/concurrency/Embedded_Connection.h>
#include <joedb/journal/Memory_File.h>

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 //
 // This sets up a configuration with a server and 2 clients.
 // Usually, clients would be in separate processes, but we can run
 // everything in the same program, for the sake of illustration.
 //
 joedb::Memory_File server_file;
 joedb::Embedded_Connection connection(server_file);

 joedb::Memory_File client1_file;
 tutorial::Client client1(connection, client1_file);

 joedb::Memory_File client2_file;
 tutorial::Client client2(connection, client2_file);

 //
 // The databases are empty. client1 will add a few cities.
 // Writing to the client database cannot occur outside of a transaction.
 // If the transaction function throws, then nothing is pushed to the server.
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
