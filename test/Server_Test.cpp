#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Writable_Database_Client.h"
#include "joedb/concurrency/Writable_Journal_Client.h"
#include "joedb/concurrency/Readonly_Journal_Client.h"
#include "joedb/concurrency/File_Connection.h"
#include "joedb/concurrency/Server_File.h"
#include "joedb/concurrency/IO_Context_Wrapper.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/File.h"

#include "Test_Sequence.h"
#include "Test_Network_Channel.h"
#include "Shared_Memory_File.h"

#include "gtest/gtest.h"

#include <thread>
#include <cstdio>
#include <csignal>

namespace joedb
{
 static constexpr bool log_to_cerr = false;
 static std::ostringstream log_stream;

 /////////////////////////////////////////////////////////////////////////////
 class Test_Server
 /////////////////////////////////////////////////////////////////////////////
 {
  public:
   std::vector<char> data;
   Shared_Memory_File file{data};
   Connection connection;
   Writable_Journal_Client client{file, connection};
   IO_Context_Wrapper io_context;

   Server server;
   std::thread thread;
   bool paused = true;

   Test_Server
   (
    bool share_client = false,
    std::chrono::seconds lock_timeout = std::chrono::seconds{0}
   ):
    server
    {
     client,
     share_client,
     io_context.io_context,
     uint16_t(0),
     lock_timeout,
     log_to_cerr ? &std::cerr : &log_stream
    }
   {
    restart();
   }

   void pause()
   {
    if (!paused)
    {
     io_context.io_context.post([&](){server.stop();});
     thread.join();
     paused = true;
    }
   }

   void restart()
   {
    if (paused)
    {
     io_context.io_context.post([this](){server.start();});
     io_context.io_context.restart();
     thread = std::thread(
      [&io_context_reference = io_context]()
      {
       io_context_reference.run();
      }
     );
     paused = false;
    }
   }

   ~Test_Server()
   {
    try
    {
     pause();
    }
    catch(...)
    {
    }
   }
 };

 /////////////////////////////////////////////////////////////////////////////
 class Port_String
 /////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::string port_string;

  public:
   Port_String(Server &server)
   {
    std::ostringstream port_stream;
    port_stream << server.get_port();
    port_string = port_stream.str();
   }

   Port_String(Test_Server &server): Port_String(server.server)
   {
   }

   [[nodiscard]] const char *get() const
   {
    return port_string.c_str();
   }
 };

 class Test_Client_Data
 {
  public:
   Test_Network_Channel channel;
   Server_Connection server_connection;

   Test_Client_Data(Buffered_File &file, Server &server):
    channel("localhost", Port_String(server).get()),
    server_connection(channel)
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
    if (log_to_cerr)
     server_connection.set_log(&std::cerr);
    else
     server_connection.set_log(nullptr);
   }

   Test_Client(Buffered_File &file, Test_Server &server):
    Test_Client(file, server.server)
   {
   }

   Writable_Journal &get_writable_journal() {return journal;}
 };

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, basic)
 /////////////////////////////////////////////////////////////////////////////
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
    journal.default_checkpoint();
   }

   try
   {
    Test_Client client(mismatched_file, server);
    FAIL() << "This should not work";
   }
   catch (const Exception &e)
   {
    EXPECT_STREQ(e.what(), "Content mismatch. The file and the connection have diverged, and cannot be synced by pulling or pushing.");
   }
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, big_read)
 /////////////////////////////////////////////////////////////////////////////
 {
  const size_t read_size = 200000000UL;
  const char * const file_name = "server.joedb";
  std::remove(file_name);

  {
   joedb::File file(file_name, Open_Mode::create_new);
   joedb::Writable_Journal journal(file);
   journal.comment(std::string(read_size, 'x'));
   journal.default_checkpoint();
  }

  joedb::File server_file(file_name, Open_Mode::read_existing);
  Connection connection;
  Readonly_Journal_Client server_client{server_file, connection};
  IO_Context_Wrapper io_context;

  const bool share_client = false;
  const uint16_t port = 0;

  Server server
  (
   server_client,
   share_client,
   io_context.io_context,
   port,
   std::chrono::seconds(0),
   log_to_cerr ? &std::cerr : &log_stream
  );

  std::thread thread([&io_context](){io_context.run();});

  {
   Test_Network_Channel channel("localhost", Port_String(server).get());
   Server_Connection server_connection(channel);
   Memory_File client_file;
   Writable_Journal_Client client(client_file, connection);
   client.pull();
  }

  io_context.io_context.post([&](){server.stop();});
  thread.join();
  std::remove(file_name);
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, concurrent_reads)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;

  const size_t comment_size = 1 << 21;
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

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, concurrent_writes)
 /////////////////////////////////////////////////////////////////////////////
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

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, multi_lock)
 /////////////////////////////////////////////////////////////////////////////
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

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, failed_push)
 /////////////////////////////////////////////////////////////////////////////
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

   EXPECT_EQ(client.get_journal().get_checkpoint_position(), 262189);
  }

  server.pause();

  EXPECT_EQ(server.client.get_journal().get_checkpoint_position(), 41);

  EXPECT_TRUE(server.file.get_size() > 1000);
  EXPECT_TRUE(server.file.get_size() < 262189);

  //
  // Connect again to complete the push
  //
  server.restart();
  {
   Test_Client client(client_file, server);
   client.push_unlock();
   EXPECT_EQ(server.client.get_journal().get_checkpoint_position(), 262189);
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, push_timeout)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server(false, std::chrono::seconds(1));

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

  EXPECT_EQ(client.get_journal().get_checkpoint_position(), 262189);
  EXPECT_EQ(server.client.get_journal().get_checkpoint_position(), 41);
  EXPECT_TRUE(server.file.get_size() > 1000);
  EXPECT_TRUE(server.file.get_size() < 262189);

  server.restart();

  client.push_unlock();
  EXPECT_EQ(server.client.get_journal().get_checkpoint_position(), 262189);
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, lock_timeout)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server(false, std::chrono::seconds(1));
  Memory_File client_file;
  Test_Client client(client_file, server);
  client.transaction([](Readable &, Writable &){
   std::this_thread::sleep_for(std::chrono::seconds(2));
  });
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, ping)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;
  Memory_File client_file;
  Test_Client client(client_file, server);
  client.server_connection.ping();
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, unlock_at_disconnection)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;
  Memory_File client_file;
  Writable_Journal journal(client_file);
  {
   Test_Client client(client_file, server);
   client.server_connection.lock_pull(journal);
  }
  {
   Test_Client client(client_file, server);
   client.server_connection.lock_pull(journal);
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, conflict)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;

  Memory_File client_file_0;
  Memory_File client_file_1;

  Test_Client client_0(client_file_0, server);
  Test_Client client_1(client_file_1, server);

  client_0.get_writable_journal().create_table("person");
  client_0.get_writable_journal().default_checkpoint();

  client_1.get_writable_journal().create_table("person");
  client_1.get_writable_journal().default_checkpoint();

  client_0.push_unlock();
  EXPECT_ANY_THROW(client_1.push_unlock());
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, shared)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server(true);

  Memory_File client_file;
  Test_Client client(client_file, server);

  Connection connection;
  Shared_Memory_File file{server.data};
  Writable_Database_Client shared_client{file, connection};

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

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, double_lock)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;

  Memory_File client_file;
  Test_Client client(client_file, server);

  Writable_Journal journal(client_file);

  client.server_connection.lock_pull(journal);
  EXPECT_ANY_THROW
  (
   client.server_connection.lock_pull(journal)
  );
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, unexpected_command)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;
  Memory_File client_file;
  Test_Client client(client_file, server);
  client.channel.write("!", 1);
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, bad_handshake)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;
  Port_String port_string(server);
  {
   Test_Network_Channel channel("localhost", port_string.get());
   channel.write("abcdefghijklm", 13);
  }
  {
   Test_Network_Channel channel("localhost", port_string.get());
   channel.write("jbcdefghijklm", 13);
  }
  {
   Test_Network_Channel channel("localhost", port_string.get());
   channel.write("jocdefghijklm", 13);
  }
  {
   Test_Network_Channel channel("localhost", port_string.get());
   channel.write("joexefghijklm", 13);
  }
  {
   Test_Network_Channel channel("localhost", port_string.get());
   channel.write("joedefghijklm", 13);
  }
  {
   Test_Network_Channel channel("localhost", port_string.get());
   channel.write("joedbfghijklm", 13);
  }
  {
   Test_Network_Channel channel("localhost", port_string.get());
   channel.write("joedb", 5);
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, push_if_ahead)
 /////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;
  Memory_File connection_file;

  {
   Writable_Journal journal(file);
   journal.comment("Hello");
   journal.default_checkpoint();
  }

  File_Connection connection(connection_file);
  Writable_Journal_Client client{file, connection};
  IO_Context_Wrapper io_context;
  const bool share_client = false;

  EXPECT_TRUE(file.get_size() > connection_file.get_size());

  Server server
  {
   client,
   share_client,
   io_context.io_context,
   uint16_t(0),
   std::chrono::seconds(0),
   log_to_cerr ? &std::cerr : &log_stream
  };

  EXPECT_EQ(file.get_size(), connection_file.get_size());
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, synchronous_backup)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server backup_server;

  Memory_File file;
  Test_Network_Channel channel("localhost", Port_String(backup_server).get());
  Server_Connection backup_server_connection(channel);
  Writable_Journal_Client backup_client(file, backup_server_connection);

  IO_Context_Wrapper io_context;

  Server server
  {
   backup_client,
   false,
   io_context.io_context,
   uint16_t(0),
   std::chrono::seconds(0),
   nullptr
  };

  std::thread thread([&io_context](){io_context.run();});

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

  io_context.io_context.post([&](){server.stop();});
  thread.join();
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, signal)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;
  std::raise(SIGINT);
  server.thread.join();
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, content_mismatch_bug)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;

  Memory_File file;

  {
   Writable_Journal journal(file);
   for (int i = 1000000; --i >= 0;)
    journal.timestamp(i);
   journal.checkpoint(Commit_Level::no_commit);
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

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, pull_queue)
 /////////////////////////////////////////////////////////////////////////////
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

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, pull_bug)
 /////////////////////////////////////////////////////////////////////////////
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

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, blob)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;

  Blob blob;

  {
   Memory_File file;
   Test_Client client(file, server);
   client.transaction([&blob](const Readable &, Writable &writable)
   {
    blob = writable.write_blob_data("glouglou");
   });
  }

  Blob blob2;
  {
   Test_Network_Channel channel("localhost", Port_String(server).get());
   Server_File file(channel);
   EXPECT_EQ(file.read_blob_data(blob), "glouglou");
   EXPECT_EQ(file.read_blob_data(blob), "glouglou");

   {
    Test_Client client(file, server);
    client.transaction([&blob2](const Readable &, Writable &writable)
    {
     blob2 = writable.write_blob_data("glagla");
    });
   }
  }

  {
   Test_Network_Channel channel("localhost", Port_String(server).get());
   Server_File file(channel);
   EXPECT_EQ(file.read_blob_data(blob), "glouglou");
   EXPECT_EQ(file.read_blob_data(blob2), "glagla");
  }

  {
   Test_Network_Channel channel("localhost", Port_String(server).get());
   Server_File file(channel);

   try
   {
    file.read_blob_data(Blob{1});
    FAIL() << "This should have failed";
   }
   catch (const std::exception &e)
   {
    EXPECT_EQ(e.what(), std::string("read_some: End of file"));
   }
  }

  {
   Test_Network_Channel channel("localhost", Port_String(server).get());
   Server_File file(channel);

   try
   {
    file.read_blob_data(Blob{123456789});
    FAIL() << "This should have failed";
   }
   catch (const std::exception &e)
   {
    EXPECT_EQ(e.what(), std::string("Trying to read past the end of file"));
   }
  }

  {
   Test_Network_Channel channel("localhost", Port_String(server).get());
   Server_File file(channel);
   EXPECT_EQ(file.read_blob_data(blob), "glouglou");
   EXPECT_EQ(file.read_blob_data(blob2), "glagla");
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, Server_File)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server;
  Test_Network_Channel channel("localhost", Port_String(server).get());
  Server_File file(channel);
  Writable_Journal_Client client(file, file);
  joedb::Blob blob;
  client.transaction([&](joedb::Writable_Journal &journal)
  {
   blob = journal.write_blob_data("blob_test");
  });
  EXPECT_EQ(file.read_blob_data(blob), "blob_test");
 }
}
