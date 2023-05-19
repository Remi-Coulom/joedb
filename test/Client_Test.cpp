#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/concurrency/Embedded_Connection.h"
#include "joedb/journal/Memory_Journal.h"
#include "joedb/Destructor_Logger.h"

#include "gtest/gtest.h"

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(Client, Interpreted_Client)
/////////////////////////////////////////////////////////////////////////////
{
 Memory_Journal server_journal;

 Memory_Journal client1_journal;
 Embedded_Connection connection1(client1_journal, server_journal);
 Interpreted_Client client1(connection1);

 Memory_Journal client2_journal;
 Embedded_Connection connection2(client2_journal, server_journal);
 Interpreted_Client client2(connection2);

 client1.transaction([](Readable &readable, Writable &writable)
 {
  writable.create_table("person");
 });

 connection1.run_while_locked([](){});

 EXPECT_EQ(0, int(client2.get_database().get_tables().size()));
 client2.pull();
 EXPECT_EQ(1, int(client2.get_database().get_tables().size()));

 client2.transaction([](Readable &readable, Writable &writable)
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
  Memory_Journal server_journal;

  Memory_Journal client1_journal;
  Embedded_Connection connection1(client1_journal, server_journal);
  Interpreted_Client client1(connection1);

  Memory_Journal client2_journal;
  Embedded_Connection connection2(client2_journal, server_journal);
  Interpreted_Client client2(connection2);

  client1.transaction([](Readable &readable, Writable &writable)
  {
   writable.create_table("person");
  });

  client2.pull();
  EXPECT_EQ(1, int(client2.get_database().get_tables().size()));

  try
  {
   client1.transaction([](Readable &readable, Writable &writable)
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
   client1.transaction([](Readable &readable, Writable &writable)
   {
    writable.create_table("country");
   });
   ADD_FAILURE() << "interrupted write should have prevented transaction";
  }
  catch (const joedb::Exception &e)
  {
   EXPECT_STREQ(e.what(), "can't pull: client is ahead of server");
  }

  client2.pull();
  EXPECT_EQ(1, int(client2.get_database().get_tables().size()));
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Client, hash)
/////////////////////////////////////////////////////////////////////////////
{
 Memory_Journal server_journal;

 {
  Memory_Journal client_journal;

  {
   Embedded_Connection connection(client_journal, server_journal);
   Interpreted_Client client(connection);
   client.transaction([](Readable &readable, Writable &writable)
   {
    writable.create_table("person");
    writable.create_table("city");
   });
  }
 }


 {
  Memory_Journal client_journal;
  client_journal.create_table("country");
  client_journal.checkpoint(Commit_Level::no_commit);
  client_journal.rewind();

  try
  {
   Embedded_Connection connection(client_journal, server_journal);
   Interpreted_Client client(connection);
   ADD_FAILURE() << "Connection with incompatible file should have failed";
  }
  catch (const joedb::Exception &e)
  {
   EXPECT_STREQ(e.what(), "Client data does not match the server");
  }
 }

 {
  Memory_Journal client_journal;
  client_journal.create_table("person");
  client_journal.checkpoint(Commit_Level::no_commit);
  client_journal.rewind();

  try
  {
   Embedded_Connection connection(client_journal, server_journal);
   Interpreted_Client client(connection);
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
 Memory_Journal server_journal;

 {
  Memory_Journal client_journal;

  //
  // Push something to the server via a connection
  //
  {
   Embedded_Connection connection(client_journal, server_journal);
   Interpreted_Client client(connection);

   client.transaction([](Readable &readable, Writable &writable)
   {
    writable.create_table("person");
   });
  }

  //
  // Make offline modifications
  //
  {
   client_journal.append();
   client_journal.create_table("city");
   client_journal.checkpoint(Commit_Level::no_commit);
  }

  //
  // Connect again, and update the server
  //
  {
   Embedded_Connection connection(client_journal, server_journal);
   Interpreted_Client client(connection);
   EXPECT_TRUE(client.get_checkpoint_difference() > 0);
   client.push_unlock();
  }
 }

 //
 // Another client connects to check the server got everything
 //
 {
  Memory_Journal client_journal;
  Embedded_Connection connection(client_journal, server_journal);
  Interpreted_Client client(connection);
  EXPECT_TRUE(client.get_checkpoint_difference() < 0);
  client.pull();
  EXPECT_EQ(client.get_database().get_tables().size(), 2ULL);
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Client, synchronization_error_at_handshake)
/////////////////////////////////////////////////////////////////////////////
{
 Memory_Journal server_journal;
 Memory_Journal client_journal;

 //
 // Push something to the server via a connection
 //
 {
  Embedded_Connection connection(client_journal, server_journal);
  Interpreted_Client client(connection);

  client.transaction([](Readable &readable, Writable &writable)
  {
   writable.create_table("person");
  });
 }

 //
 // Make offline modifications
 //
 client_journal.append();
 client_journal.create_table("city");
 client_journal.checkpoint(Commit_Level::no_commit);

 //
 // Someone else makes incompatible changes
 //
 {
  Memory_Journal client2_journal;
  Embedded_Connection connection(client2_journal, server_journal);
  Interpreted_Client client(connection);

  client.transaction([](Readable &readable, Writable &writable)
  {
   writable.create_table("company");
  });
 }

 //
 // Connect again: this should fail
 //
 try
 {
  Embedded_Connection connection(client_journal, server_journal);
  Interpreted_Client client(connection);
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
 Memory_Journal server_journal;
 Memory_Journal client_journal;
 Embedded_Connection connection(client_journal, server_journal);
 Interpreted_Client client(connection);
 client.transaction([](Readable &readable, Writable &writable){});
 client.transaction([](Readable &readable, Writable &writable){});
}
