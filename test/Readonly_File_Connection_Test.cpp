#include "joedb/concurrency/Readonly_File_Connection.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/journal/Test_File.h"

#include "gtest/gtest.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 TEST(Readonly_Connection, pull)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_File server_file;
  Writable_Journal server_journal(server_file);
  Readonly_File_Connection connection(server_file);

  Test_File client_file;
  Interpreted_Client client(connection, client_file);

  EXPECT_EQ(0, int(client.get_database().get_tables().size()));

  server_journal.create_table("person");
  client.pull();

  EXPECT_EQ(0, int(client.get_database().get_tables().size()));

  server_journal.flush();
  client.pull();

  EXPECT_EQ(0, int(client.get_database().get_tables().size()));

  server_journal.checkpoint(Commit_Level::no_commit);
  client.pull();

  EXPECT_EQ(1, int(client.get_database().get_tables().size()));
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Readonly_Connection, no_write)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_File server_file;
  Writable_Journal server_journal(server_file);
  Readonly_File_Connection connection(server_file);

  Test_File client_file;

  Interpreted_Client client(connection, client_file);

  EXPECT_ANY_THROW
  (
   client.transaction([](const Readable &readable, Writable &writable){});
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Readonly_Connection, mismatch)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_File server_file;

  {
   Writable_Journal server_journal(server_file);
   server_journal.create_table("person");
   server_journal.checkpoint(Commit_Level::no_commit);
  }

  Test_File client_file;

  {
   Writable_Journal client_journal(client_file);
   client_journal.create_table("city");
   client_journal.checkpoint(Commit_Level::no_commit);
  }
 
  Readonly_File_Connection connection(server_file);

  EXPECT_ANY_THROW
  (
   Interpreted_Client client(connection, client_file);
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Readonly_Connection, push)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_File server_file;
  Writable_Journal server_journal(server_file);

  Test_File client_file;
  {
   Writable_Journal client_journal(client_file);
   client_journal.create_table("city");
   client_journal.checkpoint(Commit_Level::no_commit);
  }
 
  Readonly_File_Connection connection(server_file);
  Interpreted_Client client(connection, client_file);

  EXPECT_ANY_THROW
  (
   client.push_unlock();
  );
 }
}
