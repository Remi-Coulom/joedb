#include "joedb/concurrency/Embedded_Connection.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/Exception.h"

#include "gtest/gtest.h"

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(Connection, Interpreted_Client)
/////////////////////////////////////////////////////////////////////////////
{
 Memory_File server_file;
 Embedded_Connection connection(server_file);

 Memory_File client1_file;
 Interpreted_Client client1(connection, client1_file);

 Memory_File client2_file;
 Interpreted_Client client2(connection, client2_file);

 client1.write_transaction
 (
  [](Readable_Writable &db)
  {
   db.create_table("person");
  }
 );

 {
  joedb::Mutex_Lock lock(connection);
 }

 EXPECT_EQ(0, int(client2.get_database().get_tables().size()));
 client2.pull();
 EXPECT_EQ(1, int(client2.get_database().get_tables().size()));

 client2.write_transaction
 (
  [](Readable_Writable &db)
  {
   db.create_table("city");
  }
 );

 EXPECT_EQ(1, int(client1.get_database().get_tables().size()));
 client1.pull();
 EXPECT_EQ(2, int(client1.get_database().get_tables().size()));
}

/////////////////////////////////////////////////////////////////////////////
TEST(Connection, Transaction_Failure)
/////////////////////////////////////////////////////////////////////////////
{
 Memory_File server_file;
 Embedded_Connection connection(server_file);

 Memory_File client1_file;
 Interpreted_Client client1(connection, client1_file);

 Memory_File client2_file;
 Interpreted_Client client2(connection, client2_file);

 client1.write_transaction
 (
  [](Readable_Writable &db)
  {
   db.create_table("person");
  }
 );

 client2.pull();
 EXPECT_EQ(1, int(client2.get_database().get_tables().size()));

 try
 {
  client1.write_transaction
  (
   [](Readable_Writable &db)
   {
    db.create_table("city");
    throw joedb::Exception("cancelled");
   }
  );
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_EQ(std::string(e.what()), "cancelled");
 }

 //
 // A cancelled transaction was not pushed to the server.
 //
 client2.pull();
 EXPECT_EQ(1, int(client2.get_database().get_tables().size()));

 //
 // The cancelled transaction was written locally, which prevents further
 // pulling from the server.
 //

 try
 {
  client1.write_transaction
  (
   [](Readable_Writable &db)
   {
    db.create_table("country");
   }
  );
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_EQ
  (
   std::string(e.what()),
   "Can't pull: failed transaction wrote to local db."
  );
 }

 client2.pull();
 EXPECT_EQ(1, int(client2.get_database().get_tables().size()));
}
