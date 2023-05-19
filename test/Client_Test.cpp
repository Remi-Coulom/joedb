#include "joedb/concurrency/Interpreted_Client_Data.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/Embedded_Connection.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/Destructor_Logger.h"

#include "gtest/gtest.h"

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(Client, Interpreted_Client)
/////////////////////////////////////////////////////////////////////////////
{
 Memory_File server_file;
 Writable_Journal server_journal(server_file);

 Memory_File client1_file;
 Interpreted_Client_Data data1(client1_file);
 Embedded_Connection connection1(data1.get_journal(), server_journal);
 Client client1(data1, connection1);

 Memory_File client2_file;
 Interpreted_Client_Data data2(client2_file);
 Embedded_Connection connection2(data2.get_journal(), server_journal);
 Client client2(data1, connection1);

 client1.transaction([&data1]()
 {
  data1.get_multiplexer().create_table("person");
 });

 connection1.run_while_locked([](){});

 EXPECT_EQ(0, int(data2.get_database().get_tables().size()));
 client2.pull();
 EXPECT_EQ(1, int(data2.get_database().get_tables().size()));

 client2.transaction([](Readable &readable, Writable &writable)
 {
  writable.create_table("city");
 });

 EXPECT_EQ(1, int(data1.get_database().get_tables().size()));
 client1.pull();
 EXPECT_EQ(2, int(data1.get_database().get_tables().size()));
}

/////////////////////////////////////////////////////////////////////////////
TEST(Client, Transaction_Failure)
/////////////////////////////////////////////////////////////////////////////
{
 {
  Memory_File server_file;
  Writable_Journal server_journal(server_file);

  Memory_File client1_file;
  Interpreted_Client_Data data1(client1_file);
  Embedded_Connection connection1(data1.get_journal(), server_journal);
  Client client1(data1, connection1);
 
  Memory_File client2_file;
  Interpreted_Client_Data data2(client2_file);
  Embedded_Connection connection2(data2.get_journal(), server_journal);
  Client client2(data1, connection1);

  client1.transaction([&data1]()
  {
   data1.get_multiplexer().create_table("person");
  });

  client2.pull();
  EXPECT_EQ(1, int(data2.get_database().get_tables().size()));

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
  EXPECT_EQ(1, int(data2.get_database().get_tables().size()));

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
  EXPECT_EQ(1, int(data2.get_database().get_tables().size()));
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Client, hash)
/////////////////////////////////////////////////////////////////////////////
{
 Memory_File server_file;
 Writable_Journal server_journal(server_file);

 {
  Memory_File client_file;
  Interpreted_Client_Data data(client_file);

  {
   Embedded_Connection connection(data.get_journal(), server_journal);
   Client client(data, connection);
   client.transaction([&data]()
   {
    data.get_multiplexer().create_table("person");
    data.get_multiplexer().create_table("city");
   });
  }
 }


 {
  Memory_File client_file;

  {
   Writable_Journal client_journal(client_file);
   client_journal.create_table("country");
   client_journal.checkpoint(Commit_Level::no_commit);
  }

  try
  {
   Interpreted_Client_Data data(client_file);
   Embedded_Connection connection(data.get_journal(), server_journal);
   Client client(data, connection);
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
   Interpreted_Client_Data data(client_file);
   Embedded_Connection connection(data.get_journal(), server_journal);
   Client client(data, connection);
   EXPECT_EQ(data.get_database().get_tables().size(), 1ULL);
   client.pull();
   EXPECT_EQ(data.get_database().get_tables().size(), 2ULL);
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
 Writable_Journal server_journal(server_file);

 {
  Memory_File client_file;
  Interpreted_Client_Data data(client_file);

  //
  // Push something to the server via a connection
  //
  {
   Embedded_Connection connection(data.get_journal(), server_journal);
   Client client(data, connection);

   client.transaction([&data]()
   {
    data.get_multiplexer().create_table("person");
   });
  }

  //
  // Make offline modifications
  //
  {
   data.get_journal().append();
   data.get_journal().create_table("city");
   data.get_journal().checkpoint(Commit_Level::no_commit);
  }

  //
  // Connect again, and update the server
  //
  {
   Embedded_Connection connection(data.get_journal(), server_journal);
   Client client(data, connection);
   EXPECT_TRUE(client.get_checkpoint_difference() > 0);
   client.push_unlock();
  }
 }

 //
 // Another client connects to check the server got everything
 //
 {
  Memory_File client_file;
  Interpreted_Client_Data data(client_file);
  Embedded_Connection connection(data.get_journal(), server_journal);
  Client client(data, connection);
  EXPECT_TRUE(client.get_checkpoint_difference() < 0);
  client.pull();
  EXPECT_EQ(data.get_database().get_tables().size(), 2ULL);
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Client, synchronization_error_at_handshake)
/////////////////////////////////////////////////////////////////////////////
{
 Memory_File server_file;
 Writable_Journal server_journal(server_file);

 Memory_File client_file;
 Interpreted_Client_Data data(client_file);

 //
 // Push something to the server via a connection
 //
 {
  Embedded_Connection connection(data.get_journal(), server_journal);
  Client client(data, connection);

  client.transaction([](Readable &readable, Writable &writable)
  {
   writable.create_table("person");
  });
 }

 //
 // Make offline modifications
 //
 data.get_journal().append();
 data.get_journal().create_table("city");
 data.get_journal().checkpoint(Commit_Level::no_commit);

 //
 // Someone else makes incompatible changes
 //
 {
  Memory_File client2_file;
  Interpreted_Client_Data data2(client2_file);
  Embedded_Connection connection(data2.get_journal(), server_journal);
  Client client(data2, connection);

  client.transaction([&data]()
  {
   data.get_multiplexer().create_table("company");
  });
 }

 //
 // Connect again: this should fail
 //
 try
 {
  Embedded_Connection connection(data.get_journal(), server_journal);
  Client client(data, connection);
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
 Writable_Journal server_journal(server_file);

 Memory_File client_journal;
 Interpreted_Client_Data data(client_journal);

 Embedded_Connection connection(data.get_journal(), server_journal);
 Client client(data, connection);
 client.transaction([](){});
 client.transaction([](){});
}
