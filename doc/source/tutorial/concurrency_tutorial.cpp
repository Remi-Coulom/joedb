#include "tutorial.h"

#include "joedb/server/Embedded_Connection.h"
#include "joedb/journal/Memory_File.h"

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 //
 // This setups a configuration with a server and 2 clients.
 // Normally, clients would be in separate processes, but we can run
 // everything in the same program for the sake of illustration.
 //
 // If you wish to connect to a remote file via ssh, you can
 // use joedb::SSH_Connection instead of joedb::Embedded_Connection.
 //
 joedb::Memory_File server_file;
 joedb::Embedded_Connection connection(server_file);

 joedb::Memory_File client1_file;
 tutorial::Client client1(connection, client1_file);

 joedb::Memory_File client2_file;
 tutorial::Client client2(connection, client2_file);

 //
 // The databases are empty. client1 will add a few cities.
 // In order to get write access, it is necessary to create a lock.
 //
 {
  tutorial::Write_Lock lock(client1);

  lock.get_database().new_city("Paris");
  lock.get_database().new_city("New York");
  lock.get_database().new_city("Tokyo");
 }

 //
 // The lock is released. Unlike the get_database() method of the lock,
 // the get_database() method of the client is read-only. Although it may
 // be a little outdated, we can always read our local copy freely.
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
