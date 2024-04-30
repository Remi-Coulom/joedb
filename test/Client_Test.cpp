#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/File_Connection.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/Destructor_Logger.h"

#include "gtest/gtest.h"

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(Client, Interpreted_Client)
/////////////////////////////////////////////////////////////////////////////
{
 Memory_File server_file;
 Memory_File client1_file;
 Memory_File client2_file;

 File_Connection connection(server_file);

 Interpreted_Client client1(connection, client1_file);
 Interpreted_Client client2(connection, client2_file);

 client1.transaction([](const Readable &readable, Writable &writable)
 {
  writable.create_table("person");
 });

 client1.transaction([](const Readable &readable, Writable &writable){});

 EXPECT_EQ(0, int(client2.get_database().get_tables().size()));
 client2.pull();
 EXPECT_EQ(1, int(client2.get_database().get_tables().size()));

 client2.transaction([](const Readable &readable, Writable &writable)
 {
  writable.create_table("city");
 });

 EXPECT_EQ(1, int(client1.get_database().get_tables().size()));
 client1.pull();
 EXPECT_EQ(2, int(client1.get_database().get_tables().size()));
}

/////////////////////////////////////////////////////////////////////////////
TEST(Client, Transaction_Failure)
/////////////////////////////////////////////////////////////////////////////
{
 {
  Memory_File server_file;
  Memory_File client1_file;
  Memory_File client2_file;

  File_Connection connection(server_file);
 
  Interpreted_Client client1(connection, client1_file);
  Interpreted_Client client2(connection, client2_file);

  client1.transaction([](const Readable &readable, Writable &writable)
  {
   writable.create_table("person");
  });

  client2.pull();
  EXPECT_EQ(1, int(client2.get_database().get_tables().size()));

  try
  {
   client1.transaction([](const Readable &readable, Writable &writable)
   {
    writable.create_table("city");
    throw joedb::Exception("cancelled");
   });
   ADD_FAILURE() << "transaction should have thrown";
  }
  catch (const joedb::Exception &e)
  {
   EXPECT_STREQ(e.what(), "cancelled");
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
   client1.transaction([](const Readable &readable, Writable &writable)
   {
    writable.create_table("country");
   });
   ADD_FAILURE() << "interrupted write should have prevented transaction";
  }
  catch (const joedb::Exception &e)
  {
   EXPECT_STREQ(e.what(), "data contains an aborted transaction");
  }

  EXPECT_ANY_THROW(client1.pull());
  EXPECT_ANY_THROW(Client_Lock{client1});

  client2.pull();
  EXPECT_EQ(1, int(client2.get_database().get_tables().size()));
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Client, hash)
/////////////////////////////////////////////////////////////////////////////
{
 Memory_File server_file;

 {
  Writable_Journal server_journal(server_file);
  server_journal.create_table("person");
  server_journal.create_table("city");
  server_journal.checkpoint(Commit_Level::no_commit);
 }

 File_Connection connection(server_file);

 {
  Memory_File client_file;

  {
   Writable_Journal client_journal(client_file);
   client_journal.create_table("country");
   client_journal.checkpoint(Commit_Level::no_commit);
  }

  try
  {
   Interpreted_Client client(connection, client_file);
   ADD_FAILURE() << "Connection with incompatible file should have failed";
  }
  catch (const joedb::Exception &e)
  {
   EXPECT_STREQ(e.what(), "Client data does not match the server");
  }
 }

 {
  Memory_File client_file;

  {
   Writable_Journal client_journal(client_file);
   client_journal.create_table("person");
   client_journal.checkpoint(Commit_Level::no_commit);
  }

  try
  {
   Interpreted_Client client(connection, client_file);
   EXPECT_EQ(client.get_database().get_tables().size(), 1ULL);
   client.pull();
   EXPECT_EQ(client.get_database().get_tables().size(), 2ULL);
  }
  catch (const joedb::Exception &e)
  {
   ADD_FAILURE() << e.what();
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Client, push)
/////////////////////////////////////////////////////////////////////////////
{
 Memory_File server_file;
 File_Connection connection(server_file);

 {
  Memory_File client_file;

  //
  // Push something to the server via a connection
  //
  {
   Interpreted_Client client(connection, client_file);

   client.transaction([](const Readable &readable, Writable &writable)
   {
    writable.create_table("person");
   });
  }

  //
  // Make offline modifications
  //
  {
   Writable_Journal journal(client_file);
   journal.append();
   journal.create_table("city");
   journal.checkpoint(Commit_Level::no_commit);
  }

  //
  // Connect again, and update the server
  //
  {
   Interpreted_Client client(connection, client_file);
   EXPECT_TRUE(client.get_checkpoint_difference() > 0);
   client.push_unlock();
  }
 }

 //
 // Another client connects to check the server got everything
 //
 {
  Memory_File client_file;
  Interpreted_Client client(connection, client_file);
  EXPECT_TRUE(client.get_checkpoint_difference() < 0);
  client.pull();
  EXPECT_EQ(client.get_database().get_tables().size(), 2ULL);
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Client, synchronization_error_at_handshake)
/////////////////////////////////////////////////////////////////////////////
{
 Memory_File server_file;
 File_Connection connection(server_file);

 Memory_File client_file;

 //
 // Push something to the server via a connection
 //
 {
  Interpreted_Client client(connection, client_file);
  client.transaction([](const Readable &readable, Writable &writable)
  {
   writable.create_table("person");
  });
 }

 //
 // Make offline modifications of the client data
 //
 {
  Writable_Journal journal(client_file);
  journal.append();
  journal.create_table("city");
  journal.checkpoint(Commit_Level::no_commit);
 }

 //
 // Someone else makes incompatible changes to the server data
 //
 {
  Memory_File client2_file;
  Interpreted_Client client2(connection, client2_file);

  client2.transaction([](const Readable &readable, Writable &writable)
  {
   writable.create_table("company");
  });
 }

 //
 // Connect again: this should fail
 //
 try
 {
  Interpreted_Client client(connection, client_file);
  ADD_FAILURE() << "connecting should have failed\n";
 }
 catch (const Exception &e)
 {
  EXPECT_STREQ(e.what(), "Client data does not match the server");
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Client, empty_transaction)
/////////////////////////////////////////////////////////////////////////////
{
 Memory_File server_file;
 Memory_File client_file;

 File_Connection connection(server_file);

 Interpreted_Client client(connection, client_file);

 client.transaction([](const Readable &readable, Writable &writable){});
 client.transaction([](const Readable &readable, Writable &writable){});
}
