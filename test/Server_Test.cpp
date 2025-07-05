#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Writable_Database_Client.h"
#include "joedb/concurrency/Writable_Journal_Client.h"
#include "joedb/concurrency/Readonly_Client.h"
#include "joedb/concurrency/File_Connection.h"
#include "joedb/concurrency/Server_File.h"
#include "joedb/concurrency/Local_Connector.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/File.h"
#include "joedb/error/Stream_Logger.h"

#include "Test_Sequence.h"
#include "Test_Local_Channel.h"
#include "Shared_Memory_File.h"

#include "gtest/gtest.h"

#include <thread>
#include <cstdio>
#include <csignal>

namespace joedb
{
 static constexpr bool log_to_cerr = false;
 static std::ostringstream log_stream;
 static Stream_Logger logger(log_to_cerr ? std::cerr : log_stream);

 ////////////////////////////////////////////////////////////////////////////
 class Test_Server
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   static constexpr std::string_view default_endpoint_path{"joedb_test.sock"};
   std::vector<char> data;
   Shared_Memory_File file{data};
   Writable_Journal_Client client{file};

   Server server;
   std::thread thread;
   bool paused = true;

   Test_Server
   (
    std::string_view endpoint_path = default_endpoint_path,
    std::chrono::seconds lock_timeout = std::chrono::seconds{0}
   ):
    server
    {
     logger,
     100,
     1,
     std::string(endpoint_path),
     client,
     lock_timeout
    },
    thread([this](){server.run();})
   {
   }

   void stop()
   {
    server.stop();
    thread.join();
   }

   ~Test_Server()
   {
    try
    {
     stop();
    }
    catch(...)
    {
    }
   }
 };

 class Test_Client_Data
 {
  public:
   Test_Local_Channel channel;
   Server_Connection server_connection;

   Test_Client_Data(Buffered_File &file, Server &server):
    channel(server.get_endpoint_path()),
    server_connection(channel, log_to_cerr ? &std::cerr : nullptr)
   {
   }
 };

 class Test_Client:
  public Test_Client_Data,
  public Writable_Database_Client
 {
  public:
   Test_Client(Buffered_File &file, Server &server):
    Test_Client_Data(file, server),
    Writable_Database_Client(file, server_connection)
   {
   }

   Test_Client(Buffered_File &file, Test_Server &server):
    Test_Client(file, server.server)
   {
   }

   Writable_Journal &get_writable_journal()
   {
    return Writable_Database_Client_Data::data_journal;
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, basic)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;

  //
  // Basic operation
  //
  Memory_File client_file_1;
  Memory_File client_file_2;

  {
   Test_Client client_1(client_file_1, server);
   Test_Client client_2(client_file_2, server);

   client_1.pull();

   client_1.transaction
   (
    [](const Readable &readable, Writable &writable)
    {
     writable.create_table("person");
    }
   );

   EXPECT_EQ(client_1.get_database().get_tables().size(), 1ULL);
   EXPECT_EQ(client_2.get_database().get_tables().size(), 0ULL);

   client_2.pull();

   EXPECT_EQ(client_2.get_database().get_tables().size(), 1ULL);

   client_1.transaction
   (
    [](const Readable &readable, Writable &writable)
    {
    }
   );
  }

  //
  // Reconnect after disconnection, with a good non-empty database
  //
  {
   Test_Client client_1(client_file_1, server);
  }

  //
  // Try reconnecting with a mismatched database
  //
  {
   Memory_File mismatched_file;

   {
    Writable_Journal journal(mismatched_file);
    journal.create_table("city");
    journal.soft_checkpoint();
   }

   try
   {
    Test_Client client(mismatched_file, server);
    FAIL() << "This should not work";
   }
   catch (const Content_Mismatch &)
   {
   }
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, big_read)
 ////////////////////////////////////////////////////////////////////////////
 {
  const size_t read_size = 200000000UL;
  const char * const file_name = "server.joedb";
  std::remove(file_name);

  {
   File file(file_name, Open_Mode::create_new);
   Writable_Journal journal(file);
   journal.comment(std::string(read_size, 'x'));
   journal.soft_checkpoint();
  }

  File server_file(file_name, Open_Mode::read_existing);
  Readonly_Client server_client{server_file};

  Server server
  (
   logger,
   100,
   1,
   "big_read.sock",
   server_client,
   std::chrono::seconds(0)
  );

  std::thread thread([&server](){server.run();});

  {
   Test_Local_Channel channel(server.get_endpoint_path());
   Server_Connection server_connection(channel);
   Memory_File client_file;
   Writable_Journal_Client client(client_file);
   client.pull();
  }

  server.stop();
  thread.join();
  std::remove(file_name);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, concurrent_reads)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;

  const size_t comment_size = 1 << 5;
  const size_t client_count = 64;

  //
  // First, push some data to the server
  //
  std::string comment(comment_size, 'x');
  for (size_t i = 0; i < comment_size; i++)
   comment[i] = char('a' + (i % 26));

  Memory_File reference_file;

  {
   Test_Client client(reference_file, server);

   client.transaction
   (
    [&comment](const Readable &readable, Writable &writable)
    {
     writable.comment(comment);
     writable.create_table("test_table");
    }
   );
  }

  //
  // Then create multiple clients and pull in parallel threads
  //
  std::vector<Memory_File> files(client_count);

  {
   std::vector<std::thread> threads;
   threads.reserve(client_count);

   for (size_t i = 0; i < client_count; i++)
   {
    threads.emplace_back
    (
     [&server, &file = files[i]]()
     {
      Test_Client client(file, server);
      client.pull();
     }
    );
   }

   for (auto &thread: threads)
    thread.join();
  }

  //
  // Check that we got the good database
  //
  for (size_t i = 0; i < client_count; i++)
   EXPECT_EQ(files[i].get_data(), reference_file.get_data());
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, concurrent_writes)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;

  const size_t client_count = 64;

  {
   std::vector<std::thread> threads;
   threads.reserve(client_count);

   for (size_t i = 0; i < client_count; i++)
   {
    threads.emplace_back
    (
     [&server, i]()
     {
      Memory_File file;
      Test_Client client(file, server);

      client.transaction
      (
       [i](const Readable &readable, Writable &writable)
       {
        const std::string table_name = "table_" + std::to_string(i);
        writable.create_table(table_name);
       }
      );
     }
    );
   }

   for (auto &thread: threads)
    thread.join();
  }

  Readonly_Journal journal(server.file);
  Database db;
  journal.replay_log(db);
  EXPECT_EQ(db.get_tables().size(), client_count);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, multi_lock)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;

  Test_Sequence sequence;

  std::thread thread_0
  (
   [&server, &sequence]()
   {
    Memory_File client_file;
    Test_Client client(client_file, server);
    sequence.increment();

    sequence.wait_for(3);
    client.transaction([&sequence](Readable &, Writable &)
    {
     sequence.send(4);
     std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    sequence.send(5);
   }
  );

  std::thread thread_1
  (
   [&server, &sequence]()
   {
    Memory_File client_file;
    Test_Client client(client_file, server);
    sequence.increment();

    sequence.wait_for(4);
    client.transaction([&sequence](Readable &, Writable &)
    {
     sequence.wait_for(5);
    });
   }
  );

  std::thread thread_2
  (
   [&server, &sequence]()
   {
    Memory_File client_file;
    Test_Client client(client_file, server);
    sequence.increment();

    sequence.wait_for(4);
    client.transaction([&sequence](Readable &, Writable &)
    {
     sequence.wait_for(5);
    });
   }
  );

  thread_0.join();
  thread_1.join();
  thread_2.join();
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, failed_push)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;

  Memory_File client_file;

  //
  // Fail during push
  //
  {
   Test_Client client(client_file, server);

   client.channel.set_fail_after_writing(1 << 16);

   try
   {
    client.transaction
    (
     [](const Readable &readable, Writable &writable)
     {
      writable.comment(std::string(1 << 18, 'x'));
     }
    );

    FAIL() << "no exception thrown";
   }
   catch (...)
   {
   }

   EXPECT_EQ(client.get_journal().get_checkpoint(), 262189);
  }

  server.stop();

  EXPECT_EQ(server.client.get_journal().get_checkpoint(), 41);

  EXPECT_TRUE(server.file.get_size() > 1000);
  EXPECT_TRUE(server.file.get_size() < 262189);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, push_timeout)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server
  (
   Test_Server::default_endpoint_path,
   std::chrono::seconds(1)
  );

  Memory_File client_file;
  Test_Client client(client_file, server);

  client.channel.set_fail_after_writing(1 << 16);
  client.channel.set_failure_is_timeout(true);

  try
  {
   client.transaction
   (
    [](const Readable &readable, Writable &writable)
    {
     writable.comment(std::string(1 << 18, 'x'));
    }
   );

   FAIL() << "no exception thrown";
  }
  catch (...)
  {
  }

  server.stop();

  EXPECT_EQ(client.get_journal().get_checkpoint(), 262189);
  EXPECT_EQ(server.client.get_journal().get_checkpoint(), 41);
  EXPECT_TRUE(server.file.get_size() > 1000);
  EXPECT_TRUE(server.file.get_size() < 262189);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, lock_timeout)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server
  (
   Test_Server::default_endpoint_path,
   std::chrono::seconds(1)
  );
  Memory_File client_file;
  Test_Client client(client_file, server);
  client.transaction([](Readable &, Writable &){
   std::this_thread::sleep_for(std::chrono::seconds(2));
  });
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, ping)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;
  Memory_File client_file;
  Test_Client client(client_file, server);
  client.server_connection.ping();
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, unlock_at_disconnection)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;
  Memory_File client_file;
  Writable_Journal journal(client_file);
  {
   Test_Client client(client_file, server);
   client.server_connection.pull
   (
    Lock_Action::lock_before,
    Data_Transfer::with_data,
    journal
   );
  }
  {
   Test_Client client(client_file, server);
   client.server_connection.pull
   (
    Lock_Action::lock_before,
    Data_Transfer::with_data,
    journal
   );
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, conflict)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;

  Memory_File client_file_0;
  Memory_File client_file_1;

  Test_Client client_0(client_file_0, server);
  Test_Client client_1(client_file_1, server);

  client_0.get_writable_journal().create_table("person");
  client_0.get_writable_journal().soft_checkpoint();

  client_1.get_writable_journal().create_table("person");
  client_1.get_writable_journal().soft_checkpoint();

  client_0.push_unlock();
  EXPECT_ANY_THROW(client_1.push_unlock());
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, shared)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;

  Memory_File client_file;
  Test_Client client(client_file, server);

  Shared_Memory_File file{server.data};
  Writable_Database_Client shared_client{file};

  client.transaction
  (
   [](const Readable &readable, Writable &writable)
   {
    writable.create_table("person");
   }
  );

  EXPECT_EQ(shared_client.get_database().get_tables().size(), 0UL);
  shared_client.pull();
  EXPECT_EQ(shared_client.get_database().get_tables().size(), 1UL);

  shared_client.transaction
  (
   [](const Readable &readable, Writable &writable)
   {
    writable.create_table("city");
   }
  );

  EXPECT_EQ(client.get_database().get_tables().size(), 1UL);
  client.pull();
  EXPECT_EQ(client.get_database().get_tables().size(), 2UL);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, double_lock)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;

  Memory_File client_file;
  Test_Client client(client_file, server);

  Writable_Journal journal(client_file);

  client.server_connection.pull
  (
   Lock_Action::lock_before,
   Data_Transfer::with_data,
   journal
  );

  EXPECT_ANY_THROW
  (
   client.server_connection.pull
   (
    Lock_Action::lock_before,
    Data_Transfer::with_data,
    journal
   );
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, unexpected_command)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;
  Memory_File client_file;
  Test_Client client(client_file, server);
  client.channel.write("!", 1);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, bad_handshake)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;
  {
   Test_Local_Channel channel(server.server.get_endpoint_path());
   channel.write("abcdefghijklm", 13);
  }
  {
   Test_Local_Channel channel(server.server.get_endpoint_path());
   channel.write("jbcdefghijklm", 13);
  }
  {
   Test_Local_Channel channel(server.server.get_endpoint_path());
   channel.write("jocdefghijklm", 13);
  }
  {
   Test_Local_Channel channel(server.server.get_endpoint_path());
   channel.write("joexefghijklm", 13);
  }
  {
   Test_Local_Channel channel(server.server.get_endpoint_path());
   channel.write("joedefghijklm", 13);
  }
  {
   Test_Local_Channel channel(server.server.get_endpoint_path());
   channel.write("joedbfghijklm", 13);
  }
  {
   Test_Local_Channel channel(server.server.get_endpoint_path());
   channel.write("joedb", 5);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, push_if_ahead)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;
  Memory_File connection_file;

  {
   Writable_Journal journal(file);
   journal.comment("Hello");
   journal.soft_checkpoint();
  }

  File_Connection connection(connection_file);
  Writable_Journal_Client client{file, connection};

  EXPECT_TRUE(file.get_size() > connection_file.get_size());

  Server server
  {
   logger,
   100,
   1,
   std::string(Test_Server::default_endpoint_path),
   client,
   std::chrono::seconds(0)
  };

  EXPECT_EQ(file.get_size(), connection_file.get_size());
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, synchronous_backup)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server backup_server;

  Memory_File file;
  Test_Local_Channel channel(backup_server.server.get_endpoint_path());
  Server_Connection backup_server_connection(channel);
  Writable_Journal_Client backup_client(file, backup_server_connection);

  Server server
  {
   logger,
   100,
   1,
   "another_server.sock",
   backup_client,
   std::chrono::seconds(0)
  };

  {
   Memory_File client_file;
   Test_Client test_client(client_file, server);

   test_client.pull();
   test_client.pull();
   test_client.pull();

   test_client.transaction
   (
    [](const Readable &readable, Writable &writable)
    {
     writable.create_table("person");
    }
   );

   {
    Readonly_Journal journal(backup_server.file);
    Database db;
    journal.replay_log(db);
    EXPECT_EQ(db.get_tables().size(), 1UL);
    EXPECT_EQ(db.get_tables().begin()->second, "person");
   }
   {
    Readonly_Journal journal(file);
    Database db;
    journal.replay_log(db);
    EXPECT_EQ(db.get_tables().size(), 1UL);
    EXPECT_EQ(db.get_tables().begin()->second, "person");
   }
   {
    Readonly_Journal journal(client_file);
    Database db;
    journal.replay_log(db);
    EXPECT_EQ(db.get_tables().size(), 1UL);
    EXPECT_EQ(db.get_tables().begin()->second, "person");
   }
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, signal)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;
  std::raise(SIGINT);
  server.thread.join();
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, content_mismatch_bug)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;

  Memory_File file;

  {
   Writable_Journal journal(file);
   for (int i = 1000000; --i >= 0;)
    journal.timestamp(i);
   journal.soft_checkpoint();
  }

  {
   Test_Client client(file, server);
   client.push_unlock();
  }

  {
   Test_Client client(file, server);
   client.push_unlock();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, pull_queue)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Sequence sequence;

  Test_Server server;

  std::thread thread([&server, &sequence]()
  {
   try
   {
    Memory_File file;
    Test_Client client(file, server);
    client.pull();
    sequence.send(1);
    client.pull(std::chrono::milliseconds(100000));
    sequence.send(2);
   }
   catch(...)
   {
   }
  });

  sequence.wait_for(1);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_EQ(1, sequence.get());

  Memory_File file;
  Test_Client client(file, server);
  client.transaction
  (
   [](const Readable &readable, Writable &writable)
   {
    writable.comment("hello");
   }
  );

  sequence.wait_for(2);
  thread.join();
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, pull_bug)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Sequence sequence;

  Test_Server server;

  std::thread thread([&server, &sequence]()
  {
   try
   {
    Memory_File file;
    Test_Client client(file, server);
    client.pull(std::chrono::milliseconds(1));
    sequence.send(1);
    sequence.wait_for(2);
    client.pull(std::chrono::milliseconds(1));
    client.pull(std::chrono::milliseconds(1));
    sequence.send(3);
   }
   catch(...)
   {
    sequence.send(4);
   }
  });

  sequence.wait_for(1);

  Memory_File file;
  Test_Client client(file, server);
  client.transaction
  (
   [](const Readable &readable, Writable &writable)
   {
    writable.comment("hello");
   }
  );

  sequence.send(2);
  thread.join();
  EXPECT_EQ(sequence.get(), 3);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, blob)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;

  Blob blob;

  {
   Memory_File file;
   Test_Client client(file, server);
   client.transaction([&blob](const Readable &, Writable &writable)
   {
    blob = writable.write_blob("glouglou");
   });
  }

  Blob blob2;

  {
   Local_Connector connector(server.server.get_endpoint_path());
   Server_File file(connector);
   EXPECT_EQ(file.read_blob(blob), "glouglou");
   EXPECT_EQ(file.read_blob(blob), "glouglou");

   {
    Test_Client client(file, server);
    client.transaction([&blob2](const Readable &, Writable &writable)
    {
     blob2 = writable.write_blob("glagla");
    });
   }
  }

  {
   Local_Connector connector(server.server.get_endpoint_path());
   Server_File file(connector);
   EXPECT_EQ(file.read_blob(blob), "glouglou");
   EXPECT_EQ(file.read_blob(blob2), "glagla");
  }

  {
   Local_Connector connector(server.server.get_endpoint_path());
   Server_File file(connector);

   try
   {
    file.read_blob(Blob{1, 1234});
    FAIL() << "This should have failed";
   }
   catch (const std::exception &e)
   {
    EXPECT_EQ(e.what(), std::string("Trying to read past the end of file"));
   }
  }

  {
   Local_Connector connector(server.server.get_endpoint_path());
   Server_File file(connector);

   try
   {
    file.read_blob(Blob{123456789, 123});
    FAIL() << "This should have failed";
   }
   catch (const std::exception &e)
   {
    EXPECT_EQ(e.what(), std::string("Trying to read past the end of file"));
   }
  }

  {
   Local_Connector connector(server.server.get_endpoint_path());
   Server_File file(connector);
   EXPECT_EQ(file.read_blob(blob), "glouglou");
   EXPECT_EQ(file.read_blob(blob2), "glagla");
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, Server_File)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;
  Local_Connector connector(server.server.get_endpoint_path());
  Server_File file(connector);
  Writable_Journal_Client client(file, file);

  const auto blob = client.transaction([](Writable_Journal &journal)
  {
   return journal.write_blob("blob_test");
  });

  EXPECT_EQ(file.read_blob(blob), "blob_test");
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, Server_File_pull)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;
  Local_Connector connector(server.server.get_endpoint_path());
  Server_File file(connector);
  Writable_Journal_Client client(file, file);

  const auto blob = client.transaction([](Writable_Journal &journal)
  {
   return journal.write_blob("blob_test");
  });

  EXPECT_EQ(file.read_blob(blob), "blob_test");

  EXPECT_EQ(client.get_journal_checkpoint(), 52);
  EXPECT_EQ(client.get_connection_checkpoint(), 52);

  Blob x_blob;

  {
   Memory_File x_file;
   Test_Client x_client(x_file, server);
   x_blob = x_client.transaction([](Readable &readable, Writable &writable)
   {
    return writable.write_blob("another one");
   });
  }

  EXPECT_EQ(client.get_journal_checkpoint(), 52);
  EXPECT_EQ(client.get_connection_checkpoint(), 52);

  EXPECT_ANY_THROW(file.read_blob(x_blob));
  client.pull();
  EXPECT_EQ(file.read_blob(x_blob), "another one");

  EXPECT_EQ(client.get_journal_checkpoint(), 65);
  EXPECT_EQ(client.get_connection_checkpoint(), 65);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, shared_abort)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;
  EXPECT_TRUE(server.client.is_shared());

  Memory_File file;
  Test_Client client(file, server);

  try
  {
   client.transaction([](Readable &readable, Writable &writable)
   {
    throw Exception("aborted transaction");
   });
  }
  catch (...)
  {
  }

  server.stop();
  EXPECT_FALSE(server.server.has_client_lock());
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server, pull_waiters)
 ////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;

  Memory_File file;

  {
   Test_Client client(file, server);
   client.pull(std::chrono::milliseconds(1));
  }

  {
   Test_Client client(file, server);
   client.transaction([](Readable &readable, Writable &writable)
   {
    writable.comment("Hello");
   });
  }
 }
}
