#include "joedb/server/Embedded_Connection.h"
#include "joedb/server/SSH_Connection.cpp"
#include "joedb/server/Interpreted_Client.h"
#include "joedb/journal/Memory_File.h"

#include "gtest/gtest.h"

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(Connection, Interpreted_Client)
/////////////////////////////////////////////////////////////////////////////
{
#if JOEDB_HAS_SSH
 SSH_Robust_Connection connection
 (
  "rcoulom",
  "www.remi-coulom.fr",
  22,
  "server.joedb",
  true,
  0
 );
#else
 Memory_File server_file;
 Embedded_Connection connection(server_file);
#endif

 Memory_File client1_file;
 Interpreted_Client client1(connection, client1_file);

 Memory_File client2_file;
 Interpreted_Client client2(connection, client2_file);

 {
  Interpreted_Lock lock(client1);
  lock.get_database().create_table("person");
 }

 EXPECT_EQ(0, int(client2.get_database().get_tables().size()));
 client2.pull();
 EXPECT_EQ(1, int(client2.get_database().get_tables().size()));

 {
  Interpreted_Lock lock(client2);
  lock.get_database().create_table("city");
 }

 EXPECT_EQ(1, int(client1.get_database().get_tables().size()));
 client1.pull();
 EXPECT_EQ(2, int(client1.get_database().get_tables().size()));
}
