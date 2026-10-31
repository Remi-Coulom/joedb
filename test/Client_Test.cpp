#include "joedb/concurrency/Writable_Database_Client.h"
#include "joedb/concurrency/Readonly_Database_Client.h"
#include "joedb/concurrency/Writable_Journal_Client.h"
#include "joedb/concurrency/File_Connection.h"
#include "joedb/journal/Memory_File.h"

#include "Shared_Memory_File.h"

#include "gtest/gtest.h"
#include <ctime>

namespace joedb
{
 TEST(Client, transaction_writes_enabled_markers_and_syncs)
 {
  class Synced_Memory_File: public Memory_File
  {
   public:
    unsigned sync_count = 0;
    void sync() override {++sync_count;}
  } file;
  Writable_Journal_Client client(file);
  client.set_valid_data(true);
  client.set_timestamp(true);
  client.set_hard_checkpoint(true);
  const unsigned previous_syncs = file.sync_count;
  const int64_t before = std::time(nullptr);
  client.transaction([](Writable_Journal &journal) {journal.comment("transaction");});
  const int64_t after = std::time(nullptr);
  EXPECT_GT(file.sync_count, previous_syncs);
  class Markers: public Dummy_Writable
  {
   public:
    unsigned valid_count = 0;
    unsigned timestamp_count = 0;
    int64_t time = 0;
    std::string text;
    void valid_data() override {++valid_count;}
    void timestamp(int64_t value) override {++timestamp_count; time = value;}
    void comment(const std::string &value) override {text = value;}
  } markers;
  Readonly_Journal journal(file);
  journal.replay_log(markers);
  EXPECT_EQ(markers.valid_count, 1);
  EXPECT_EQ(markers.timestamp_count, 1);
  EXPECT_GE(markers.time, before);
  EXPECT_LE(markers.time, after);
  EXPECT_EQ(markers.text, "transaction");
 }

 TEST(File_Connection, rejects_push_from_stale_checkpoint)
 {
  Memory_File server_file;
  Writable_Journal server(server_file);
  Journal_Connection connection(server);
  EXPECT_FALSE(connection.is_pullonly());
  Memory_File client_file;
  Writable_Journal client(client_file);
  client.pull_from(server);
  const int64_t initial_checkpoint = server.get_checkpoint();
  client.comment("client update");
  client.soft_checkpoint();
  server.comment("concurrent server update");
  server.soft_checkpoint();
  const std::string saved = server_file.get_data();
  EXPECT_THROW(connection.push(client, initial_checkpoint, client.get_checkpoint(),
   Unlock_Action::unlock_after), Exception);
  EXPECT_EQ(server_file.get_data(), saved);
 }

 TEST(File_Connection, fast_content_check_compares_tail)
 {
  Memory_File source;
  {
   Writable_Journal journal(source);
   journal.comment(std::string(10000, 'x'));
   journal.soft_checkpoint();
  }
  Memory_File changed;
  changed.get_data() = source.get_data();
  Readonly_Journal source_journal(source);
  Readonly_Journal changed_journal(changed);
  Pullonly_Journal_Connection connection(source_journal);
  EXPECT_NO_THROW(connection.handshake(changed_journal, Content_Check::fast));
  changed.get_data().back() = 'y';
  EXPECT_THROW(connection.handshake(changed_journal, Content_Check::fast), Content_Mismatch);
 }

 TEST(File_Connection, readonly_rejects_lock_and_push)
 {
  Memory_File source;
  Writable_Journal journal(source);
  journal.comment("source");
  journal.soft_checkpoint();
  Pullonly_Journal_Connection connection(journal);
  EXPECT_TRUE(connection.is_pullonly());
  Memory_File target;
  Writable_Journal target_journal(target);
  EXPECT_THROW(connection.pull(Lock_Action::lock_before, Data_Transfer::without_data,
   target_journal, std::chrono::milliseconds(0)), Exception);
  EXPECT_THROW(connection.push(target_journal, Header::ssize, Header::ssize,
   Unlock_Action::unlock_after), Exception);
 }

 TEST(File_Connection, full_content_check_detects_middle_difference)
 {
  Memory_File source;
  {
   Writable_Journal journal(source);
   journal.comment(std::string(10000, 'x'));
   journal.soft_checkpoint();
  }
  Memory_File changed;
  changed.get_data() = source.get_data();
  changed.get_data()[5000] = 'y';
  Readonly_Journal source_journal(source);
  Readonly_Journal changed_journal(changed);
  Pullonly_Journal_Connection connection(source_journal);
  EXPECT_THROW(connection.handshake(changed_journal, Content_Check::full), Content_Mismatch);
  EXPECT_NO_THROW(connection.handshake(changed_journal, Content_Check::none));
  changed.get_data()[Header::size + 10] = 'z';
  EXPECT_THROW(connection.handshake(changed_journal, Content_Check::fast), Content_Mismatch);
 }


 /////////////////////////////////////////////////////////////////////////////
 TEST(Client, read_journal_failure_unlocks_connection)
 /////////////////////////////////////////////////////////////////////////////
 {
  class Tracking_Connection: public Connection
  {
   public:
    bool locked = false;
    bool fail_unlock = false;
    int unlock_count = 0;

    int64_t pull
    (
     Lock_Action lock_action,
     Data_Transfer,
     Writable_Journal &journal,
     std::chrono::milliseconds
    ) override
    {
     locked = bool(lock_action);
     return journal.get_checkpoint();
    }

    void unlock() override
    {
     ++unlock_count;
     locked = false;
     if (fail_unlock)
      throw Exception("unlock failed");
    }
  };

  class Failing_Client: public Writable_Journal_Client
  {
   public:
    using Writable_Journal_Client::Writable_Journal_Client;

    void read_journal() override
    {
     throw Exception("replay failed");
    }
  };

  for (bool fail_unlock: {false, true})
  {
   for (int operation = 0; operation < 3; ++operation)
   {
    SCOPED_TRACE(fail_unlock);
    SCOPED_TRACE(operation);
    Memory_File file;
    Tracking_Connection connection;
    connection.fail_unlock = fail_unlock;
    Failing_Client client(file, connection);

    try
    {
     if (operation == 0)
      client.transaction([](Writable_Journal &) {FAIL() << "callback executed";});
     else if (operation == 1)
      client.transaction([](Writable_Journal &) {ADD_FAILURE() << "callback executed"; return 1;});
     else
     {
      Client_Lock lock(client);
     }
     ADD_FAILURE() << "replay should have thrown";
    }
    catch (const Exception &e)
    {
     EXPECT_STREQ(e.what(), "replay failed");
    }

    EXPECT_EQ(connection.unlock_count, 1);
    EXPECT_FALSE(connection.locked);
   }
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Client, transaction_preserves_exception_when_unlock_fails)
 /////////////////////////////////////////////////////////////////////////////
 {
  class Failing_Connection: public Connection
  {
   public:
    int unlock_count = 0;

    void unlock() override
    {
     ++unlock_count;
     throw Exception("unlock failed");
    }
  };

  for (bool returns_value: {false, true})
  {
   SCOPED_TRACE(returns_value);
   Memory_File file;
   Failing_Connection connection;
   Writable_Journal_Client client(file, connection);

   try
   {
    if (returns_value)
     client.transaction([](Writable_Journal &) -> int
     {
      throw Exception("transaction failed");
     });
    else
     client.transaction([](Writable_Journal &)
     {
      throw Exception("transaction failed");
     });
    ADD_FAILURE() << "transaction should have thrown";
   }
   catch (const Exception &e)
   {
    EXPECT_STREQ(e.what(), "transaction failed");
   }

   EXPECT_EQ(connection.unlock_count, 1);
  }
 }

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
    EXPECT_STREQ(e.what(), "locking journal with uncheckpointed data (try rollback?)");
   }

   EXPECT_ANY_THROW(client1.pull());
   EXPECT_ANY_THROW(Client_Lock{client1});

   client2.pull();
   EXPECT_EQ(1, int(client2.get_database().get_tables().size()));
  }
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

  EXPECT_EQ(client.get_journal().get_position(), 57);
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
    Writable_Database_Client client2(file);
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

 ////////////////////////////////////////////////////////////////////////////
 TEST(Client, pull_when_ahead)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File server_file;
  Memory_File client_file;

  {
   Writable_Journal journal(client_file);
   journal.comment("client");
   journal.soft_checkpoint();
  }

  File_Connection connection(server_file);
  Writable_Journal_Client client(client_file, connection);
  EXPECT_NE(server_file.get_size(), client_file.get_size());

  EXPECT_ANY_THROW(client.pull());
  EXPECT_ANY_THROW(client.transaction([](Writable_Journal &journal){}));
  EXPECT_ANY_THROW(Client_Lock(client).checkpoint_and_push_unlock());
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Client, no_conflict_when_shared)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File server_file;
  Memory_File client_file(Open_Mode::shared_write);

  File_Connection connection(server_file);

  Writable_Journal_Client client1(client_file, connection);
  Writable_Journal_Client client2(client_file, connection);

  client1.transaction(
   [](Writable_Journal &journal)
   {
    EXPECT_EQ(journal.get_checkpoint(), 41);
    journal.comment("Hi!");
   }
  );

  client2.transaction(
   [](Writable_Journal &journal)
   {
    EXPECT_EQ(journal.get_checkpoint(), 46);
    journal.comment("Ha!");
   }
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Client, conflict_when_shared)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File server_file;
  Memory_File client_file(Open_Mode::shared_write);

  File_Connection connection(server_file);

  Writable_Journal_Client client1(client_file, Connection::dummy);
  Writable_Journal_Client client2(client_file, connection);

  client1.transaction(
   [](Writable_Journal &journal)
   {
    EXPECT_EQ(journal.get_checkpoint(), 41);
    journal.comment("Hi!");
   }
  );

  EXPECT_THROW
  (
   client2.transaction(
    [](Writable_Journal &journal)
    {
     EXPECT_EQ(journal.get_checkpoint(), 46);
     journal.comment("Ha!");
    }
   ),
   Content_Mismatch
  );
 }
}
