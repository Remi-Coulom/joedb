#include "joedb/server/Embedded_Server.h"
#include "joedb/server/Interpreted_Client.h"
#include "joedb/journal/File.h"

#include "gtest/gtest.h"

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(Server, Interpreted_Client)
/////////////////////////////////////////////////////////////////////////////
{
 File server_file("server.joedb", Open_Mode::create_new);
 Embedded_Server server(server_file);

 File client1_file("client1.joedb", Open_Mode::create_new);
 Interpreted_Client client1(server, client1_file);

 File client2_file("client2.joedb", Open_Mode::create_new);
 Interpreted_Client client2(server, client2_file);

 {
  Interpreted_Write_Lock lock(client1);
  lock.get_writable().create_table("person");
 }

 EXPECT_EQ(0ULL, client2.get_readable().get_tables().size());
 client2.pull();
 EXPECT_EQ(1ULL, client2.get_readable().get_tables().size());

 {
  Interpreted_Write_Lock lock(client2);
  lock.get_writable().create_table("city");
 }

 EXPECT_EQ(1ULL, client1.get_readable().get_tables().size());
 client1.pull();
 EXPECT_EQ(2ULL, client1.get_readable().get_tables().size());
}
