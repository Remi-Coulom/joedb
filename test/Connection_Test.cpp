#include "joedb/concurrency/Embedded_Connection.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/concurrency/Mutex_Lock.h"
#include "joedb/journal/Memory_File.h"

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

 client1.write_transaction([](Readable_Writable &db)
 {
  db.create_table("person");
 });

 client2.pull();
 EXPECT_EQ(1, int(client2.get_database().get_tables().size()));

 try
 {
  client1.write_transaction([](Readable_Writable &db)
  {
   db.create_table("city");
   db.checkpoint(Commit_Level::no_commit);
   throw joedb::Exception("cancelled");
  });
  FAIL() << "transaction should have thrown";
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
  client1.write_transaction([](Readable_Writable &db)
  {
   db.create_table("country");
  });
  FAIL() << "interrupted write should have prevented transaction";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ
  (
   e.what(),
   "Can't pull: failed transaction wrote to local db."
  );
 }

 client2.pull();
 EXPECT_EQ(1, int(client2.get_database().get_tables().size()));
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
   client.write_transaction([](Readable_Writable &db)
   {
    db.create_table("person");
    db.create_table("city");
   });
  }
 }

 {
  Memory_File client_file(Open_Mode::create_new);

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
   FAIL() << "Connection with incompatible file should have failed";
  }
  catch (const joedb::Exception &e)
  {
   EXPECT_STREQ(e.what(), "Hash mismatch");
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
   FAIL() << e.what();
  }
 }
}
