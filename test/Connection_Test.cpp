#include "joedb/concurrency/Embedded_Connection.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/Destructor_Logger.h"

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

 client1.transaction([](Readable &readable, Writable &writable)
 {
  writable.create_table("person");
 });

 connection.run_while_locked([](){});

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
TEST(Connection, Transaction_Failure)
/////////////////////////////////////////////////////////////////////////////
{
 {
  Memory_File server_file;
  Embedded_Connection connection(server_file);

  Memory_File client1_file;
  Interpreted_Client client1(connection, client1_file);

  Memory_File client2_file;
  Interpreted_Client client2(connection, client2_file);

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
   EXPECT_STREQ(e.what(), "Local journal contains cancelled transaction");
  }

  client2.pull();
  EXPECT_EQ(1, int(client2.get_database().get_tables().size()));

  joedb::Destructor_Logger::remove_logger();
 }

 joedb::Destructor_Logger::set_logger();
}

/////////////////////////////////////////////////////////////////////////////
TEST(Connection, hash)
/////////////////////////////////////////////////////////////////////////////
{
 Memory_File server_file;
 Embedded_Connection connection(server_file);

 {
  Memory_File client_file;

  {
   Interpreted_Client client(connection, client_file);
   client.transaction([](Readable &readable, Writable &writable)
   {
    writable.create_table("person");
    writable.create_table("city");
   });
  }
 }

 {
  Memory_File client_file;

  {
   Writable_Journal journal(client_file);
   journal.create_table("country");
   journal.checkpoint(Commit_Level::no_commit);
  }

  try
  {
   client_file.set_position(0);
   client_file.set_mode(Open_Mode::write_existing);
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
   client_file.set_position(0);
   client_file.set_mode(Open_Mode::write_existing);
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
TEST(Connection, synchronize_server_at_handshake)
/////////////////////////////////////////////////////////////////////////////
{
 Memory_File server_file;
 Embedded_Connection connection(server_file);

 {
  Memory_File client_file;

  //
  // Push something to the server via a connection
  //
  {
   Interpreted_Client client(connection, client_file);

   client.transaction([](Readable &readable, Writable &writable)
   {
    writable.create_table("person");
   });
  }

  //
  // Make offline modifications
  //
  client_file.set_mode(Open_Mode::write_existing);
  {
   Writable_Journal journal(client_file);
   journal.seek(journal.get_checkpoint_position());
   journal.create_table("city");
   journal.checkpoint(Commit_Level::no_commit);
  }

  //
  // Connect again: this should update the server
  //
  {
   Interpreted_Client client(connection, client_file);
  }
 }

 //
 // Another client connects to check the server got everything
 //
 {
  Memory_File client_file;
  Interpreted_Client client(connection, client_file);
  client.pull();
  EXPECT_EQ(client.get_database().get_tables().size(), 2ULL);
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Connection, synchronization_error_at_handshake)
/////////////////////////////////////////////////////////////////////////////
{
 Memory_File server_file;
 Embedded_Connection connection(server_file);

 Memory_File client_file;

 //
 // Push something to the server via a connection
 //
 {
  Interpreted_Client client(connection, client_file);

  client.transaction([](Readable &readable, Writable &writable)
  {
   writable.create_table("person");
  });
 }

 //
 // Make offline modifications
 //
 client_file.set_mode(Open_Mode::write_existing);
 {
  Writable_Journal journal(client_file);
  journal.seek(journal.get_checkpoint_position());
  journal.create_table("city");
  journal.checkpoint(Commit_Level::no_commit);
 }

 //
 // Someone else makes incompatible changes
 //
 {
  Memory_File client2_file;
  Interpreted_Client client(connection, client2_file);

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
  Interpreted_Client client(connection, client_file);
  ADD_FAILURE() << "connecting should have failed\n";
 }
 catch (const Exception &e)
 {
  EXPECT_STREQ(e.what(), "Client data does not match the server");
 }
}
