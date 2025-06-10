#include "joedb/concurrency/Writable_Database_Client.h"
#include "joedb/concurrency/Readonly_Database_Client.h"
#include "joedb/concurrency/Writable_Journal_Client.h"
#include "joedb/concurrency/File_Connection.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/File_View.h"

#include "Shared_Memory_File.h"

#include "gtest/gtest.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 TEST(Client, Writable_Database_Client)
 /////////////////////////////////////////////////////////////////////////////
 {
  Memory_File server_file;
  Memory_File client1_file;
  Memory_File client2_file;

  File_Connection connection(server_file);

  Writable_Database_Client client1(client1_file, connection);
  Writable_Database_Client client2(client2_file, connection);

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
 TEST(Client, Readonly_Database_Client)
 /////////////////////////////////////////////////////////////////////////////
 {
  std::vector<char> data;
  Shared_Memory_File file1(data);
  Shared_Memory_File file2(data);
  Shared_Memory_File file3(data);

  Writable_Database_Client writable_client(file1);
  Readonly_Database_Client database_client(file2);
  Readonly_Client journal_client(file3);

  writable_client.transaction([](const Readable &readable, Writable &writable)
  {
   writable.create_table("person");
  });

  EXPECT_EQ(1, int(writable_client.get_database().get_tables().size()));
  EXPECT_EQ(0, int(database_client.get_database().get_tables().size()));

  database_client.pull();

  EXPECT_EQ(1, int(database_client.get_database().get_tables().size()));

  EXPECT_TRUE(journal_client.get_journal_checkpoint() < database_client.get_journal_checkpoint());

  journal_client.pull();

  EXPECT_TRUE(journal_client.get_journal_checkpoint() == database_client.get_journal_checkpoint());
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

   Writable_Database_Client client1(client1_file, connection);
   Writable_Database_Client client2(client2_file, connection);

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
     throw Exception("cancelled");
    });
    ADD_FAILURE() << "transaction should have thrown";
   }
   catch (const Exception &e)
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
   catch (const Exception &e)
   {
    EXPECT_STREQ(e.what(), "locking journal with uncheckpointed data");
   }

   EXPECT_ANY_THROW(client1.pull());
   EXPECT_ANY_THROW(Client_Lock{client1});

   client2.pull();
   EXPECT_EQ(1, int(client2.get_database().get_tables().size()));
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Client, pull_when_ahead)
 /////////////////////////////////////////////////////////////////////////////
 {
  Memory_File client_file;

  {
   Writable_Journal journal(client_file);
   journal.comment("Hello");
   journal.soft_checkpoint();
  }

  Memory_File server_file;
  File_Connection connection(server_file);
  Writable_Database_Client client(client_file, connection);

  client.pull();
  client.push_unlock();

  EXPECT_EQ(server_file.get_size(), client_file.get_size());
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Client, Client_Lock)
 /////////////////////////////////////////////////////////////////////////////
 {
  Memory_File client_file;
  Memory_File server_file;
  File_Connection connection(server_file);
  Writable_Journal_Client client(client_file, connection);

  {
   Writable_Journal_Client_Lock lock(client);

   // None of the commented-out blocks should compile

 #if 0
  Client_Lock lock_copy(lock);
 #endif

 #if 0
  Client_Lock lock_copy = std::move(lock);
 #endif

 #if 0
  Client_Lock lock_copy(client);
  lock_copy = lock;
 #endif

   lock.get_journal().comment("Hello");
   lock.get_journal().soft_checkpoint();
   EXPECT_EQ(server_file.get_size(), 41);
   lock.checkpoint_and_push();
   EXPECT_EQ(server_file.get_size(), 48);
   lock.get_journal().comment("Hi");
   EXPECT_EQ(server_file.get_size(), 48);
   lock.checkpoint_and_push_unlock();
  }

  EXPECT_EQ(server_file.get_size(), 52);

  try
  {
   Writable_Journal_Client_Lock lock(client);
   lock.get_journal().comment("Bye");
   throw Exception("exception");
  }
  catch (...)
  {
  }

  client_file.flush();
  EXPECT_EQ(client_file.get_size(), 57);
  EXPECT_EQ(client.get_journal_checkpoint(), 52);
  EXPECT_EQ(server_file.get_size(), 52);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Client, hash)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File server_file;

  {
   Writable_Journal server_journal(server_file);
   server_journal.create_table("person");
   server_journal.create_table("city");
   server_journal.soft_checkpoint();
  }

  File_Connection connection(server_file);

  {
   Memory_File client_file;

   {
    Writable_Journal client_journal(client_file);
    client_journal.create_table("country");
    client_journal.soft_checkpoint();
   }

   try
   {
    Writable_Database_Client client(client_file, connection);
    ADD_FAILURE() << "Connection with incompatible file should have failed";
   }
   catch (const Content_Mismatch &)
   {
   }
  }

  {
   Memory_File client_file;

   {
    Writable_Journal client_journal(client_file);
    client_journal.create_table("person");
    client_journal.soft_checkpoint();
   }

   try
   {
    Writable_Database_Client client(client_file, connection);
    EXPECT_EQ(client.get_database().get_tables().size(), 1ULL);
    client.pull();
    EXPECT_EQ(client.get_database().get_tables().size(), 2ULL);
   }
   catch (const Exception &e)
   {
    ADD_FAILURE() << e.what();
   }
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Client, push)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File server_file;
  File_Connection connection(server_file);

  {
   Memory_File client_file;

   //
   // Push something to the server via a connection
   //
   {
    Writable_Database_Client client(client_file, connection);

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
    journal.skip_directly_to(journal.get_checkpoint());
    journal.create_table("city");
    journal.soft_checkpoint();
   }

   //
   // Connect again, and update the server
   //
   {
    Writable_Database_Client client(client_file, connection);
    EXPECT_TRUE(client.get_checkpoint_difference() > 0);
    client.push_unlock();
   }
  }

  //
  // Another client connects to check the server got everything
  //
  {
   Memory_File client_file;
   Writable_Database_Client client(client_file, connection);
   EXPECT_TRUE(client.get_checkpoint_difference() < 0);
   client.pull();
   EXPECT_EQ(client.get_database().get_tables().size(), 2ULL);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Client, synchronization_error_at_handshake)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File server_file;
  File_Connection connection(server_file);

  Memory_File client_file;

  //
  // Push something to the server via a connection
  //
  {
   Writable_Database_Client client(client_file, connection);
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
   journal.skip_directly_to(journal.get_checkpoint());
   journal.create_table("city");
   journal.soft_checkpoint();
  }

  //
  // Someone else makes incompatible changes to the server data
  //
  {
   Memory_File client2_file;
   Writable_Database_Client client2(client2_file, connection);

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
   Writable_Database_Client client(client_file, connection);
   ADD_FAILURE() << "connecting should have failed\n";
  }
  catch (const Content_Mismatch &)
  {
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Client, empty_transaction)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File server_file;
  Memory_File client_file;

  File_Connection connection(server_file);

  Writable_Database_Client client(client_file, connection);

  client.transaction([](const Readable &readable, Writable &writable){});
  client.transaction([](const Readable &readable, Writable &writable){});
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Client, push_until)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File client_file;

  int64_t initial;
  int64_t after_person;
  int64_t after_city;

  {
   Writable_Journal journal(client_file);
   initial = journal.get_checkpoint();
   journal.create_table("person");
   journal.soft_checkpoint();
   after_person = journal.get_checkpoint();
   journal.create_table("city");
   journal.soft_checkpoint();
   after_city = journal.get_checkpoint();
  }

  EXPECT_TRUE(after_person > initial);
  EXPECT_TRUE(after_city > after_person);

  Readonly_Journal client_journal(client_file);

  Memory_File server_file;

  File_Connection connection(server_file);
  int64_t server_checkpoint = connection.handshake
  (
   client_journal,
   Content_Check::fast
  );

  EXPECT_EQ(server_checkpoint, initial);

  server_checkpoint = connection.push
  (
   client_journal,
   server_checkpoint,
   after_person,
   Unlock_Action::unlock_after
  );

  EXPECT_EQ(server_file.get_size(), after_person);
  EXPECT_EQ(server_checkpoint, after_person);

  server_checkpoint = connection.push
  (
   client_journal,
   server_checkpoint,
   after_city,
   Unlock_Action::unlock_after
  );

  EXPECT_EQ(server_file.get_size(), after_city);
  EXPECT_EQ(server_checkpoint, after_city);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Client, pull_from_memory)
 ////////////////////////////////////////////////////////////////////////////
 {
  for (int shared = 2; --shared >= 0;)
  {
   Memory_File file
   (
    shared
    ? joedb::Open_Mode::shared_write
    : joedb::Open_Mode::create_new
   );
   Writable_Database_Client client(file);
   client.transaction([](Readable &readable, Writable &writable)
   {
    writable.create_table("person");
   });

   EXPECT_EQ(client.get_database().get_tables().size(), 1);

   {
    File_View file_view(file);
    Writable_Database_Client client2(file_view);
    client2.transaction([](Readable &readable, Writable &writable)
    {
     writable.create_table("city");
    });
    EXPECT_EQ(client2.get_database().get_tables().size(), 2);
   }

   EXPECT_EQ(client.get_database().get_tables().size(), 1);

   client.pull();

   if (shared)
    EXPECT_EQ(client.get_database().get_tables().size(), 2);
   else
    EXPECT_EQ(client.get_database().get_tables().size(), 1);
  }
 }
}
