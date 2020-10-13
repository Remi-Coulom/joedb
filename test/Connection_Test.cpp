#include "joedb/server/Embedded_Connection.h"
#include "joedb/server/System_SSH_Connection.cpp"
#include "joedb/server/SSH_Connection.cpp"
#include "joedb/server/Interpreted_Client.h"
#include "joedb/journal/File.h"

#include "gtest/gtest.h"

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(Connection, Interpreted_Client)
/////////////////////////////////////////////////////////////////////////////
{
#if 0
 System_SSH_Connection connection("www.remi-coulom.fr", "server.joedb");
#elif 1
 SSH_Connection connection("rcoulom", "www.remi-coulom.fr", 22, "server.joedb");
#else
 File server_file("server.joedb", Open_Mode::create_new);
 Embedded_Connection connection(server_file);
#endif

 File client1_file("client1.joedb", Open_Mode::create_new);
 Interpreted_Client client1(connection, client1_file);

 File client2_file("client2.joedb", Open_Mode::create_new);
 Interpreted_Client client2(connection, client2_file);

 {
  Interpreted_Write_Lock lock(client1);
  lock.get_writable().create_table("person");
 }

 EXPECT_EQ(0, int(client2.get_readable().get_tables().size()));
 client2.pull();
 EXPECT_EQ(1, int(client2.get_readable().get_tables().size()));

 {
  Interpreted_Write_Lock lock(client2);
  lock.get_writable().create_table("city");
 }

 EXPECT_EQ(1, int(client1.get_readable().get_tables().size()));
 client1.pull();
 EXPECT_EQ(2, int(client1.get_readable().get_tables().size()));
}
