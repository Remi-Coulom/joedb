#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/Writable_Journal_Client_Data.h"
#include "joedb/concurrency/Local_Connection.h"
#include "joedb/journal/Memory_File.h"

#include "Test_Network_Channel.h"

#include "gtest/gtest.h"

#include <thread>
#include <fstream>

namespace joedb
{
 static constexpr bool log_to_cerr = false;
 static std::ostringstream log_stream;

 /////////////////////////////////////////////////////////////////////////////
 class Test_Server
 /////////////////////////////////////////////////////////////////////////////
 {
  public:
   Memory_File file;
   Writable_Journal_Client_Data client_data{file};
   std::unique_ptr<Connection> connection;
   Client client{client_data, *connection};
   net::io_context io_context;

   Server server;
   std::thread thread;
   std::string port_string;
   bool paused = true;

  public:
   Test_Server
   (
    bool share_client,
    std::chrono::seconds lock_timeout
   ):
    file(share_client ? Open_Mode::shared_write : Open_Mode::create_new),
    connection(share_client ? new Local_Connection() : new Connection()),
    server
    {
     client,
     share_client,
     io_context,
     uint16_t(0),
     lock_timeout,
     log_to_cerr ? &std::cerr : &log_stream
    }
   {
    std::ostringstream port_stream;
    port_stream << server.get_port();
    port_string = port_stream.str();
    restart();
   }

   const char *get_port() const
   {
    return port_string.c_str();
   }

   void set_log(std::ostream *out)
   {
    server.set_log(out);
   }

   void pause()
   {
    if (!paused)
    {
     server.pause();
     thread.join();
     paused = true;
    }
   }

   void restart()
   {
    if (paused)
    {
     server.restart();
     thread = std::thread(
      [&io_context = io_context]()
      {
       io_context.run();
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
 class Test_Client
 /////////////////////////////////////////////////////////////////////////////
 {
  public:
   Test_Network_Channel channel;
   Server_Connection connection;
   Interpreted_Client client;

  public:
   Test_Client(Test_Server &server, Generic_File &file):
    channel("localhost", server.get_port()),
    connection(channel, nullptr),
    client(connection, file)
   {
   }
 };

 /////////////////////////////////////////////////////////////////////////////
 class Test_Sequence
 /////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::mutex mutex;
   std::condition_variable condition;
   int n = 0;

   void log()
   {
    // std::cerr << "n = " << n << '\n';
   }

  public:
   void send(int new_n)
   {
    {
     std::unique_lock<std::mutex> lock(mutex);
     n = new_n;
     log();
    }
    condition.notify_all();
   }

   void increment()
   {
    {
     std::unique_lock<std::mutex> lock(mutex);
     n++;
     log();
    }
    condition.notify_all();
   }

   void wait_for(int awaited_n)
   {
    std::unique_lock<std::mutex> lock(mutex);
    while (awaited_n > n)
     condition.wait(lock);
   }
 };

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, basic)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server(false, std::chrono::seconds(0));

  //
  // Basic operation
  //
  Memory_File client_file_1;
  Memory_File client_file_2;

  {
   Test_Client client_1(server, client_file_1);
   Test_Client client_2(server, client_file_2);

   client_1.client.pull();

   client_1.client.transaction
   (
    [](const Readable &readable, Writable &writable)
    {
     writable.create_table("person");
    }
   );

   EXPECT_EQ(client_1.client.get_database().get_tables().size(), 1ULL);
   EXPECT_EQ(client_2.client.get_database().get_tables().size(), 0ULL);

   client_2.client.pull();

   EXPECT_EQ(client_2.client.get_database().get_tables().size(), 1ULL);

   client_1.client.transaction
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
   Test_Client client_1(server, client_file_1);
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
    Test_Client client(server, mismatched_file);
    FAIL() << "This should not work";
   }
   catch (const Exception &e)
   {
    EXPECT_STREQ(e.what(), "Client data does not match the server");
   }
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, concurrent_reads)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server(false, std::chrono::seconds(0));

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
   Test_Client client(server, reference_file);

   client.client.transaction
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
      Test_Client client(server, file);
      client.client.pull();
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
 TEST(Server, multi_lock)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server(false, std::chrono::seconds(0));

  Test_Sequence sequence;

  std::thread thread_0
  (
   [&server, &sequence]()
   {
    Memory_File client_file;
    Test_Client client(server, client_file);
    sequence.increment();

    sequence.wait_for(3);
    client.connection.lock(client.client.get_readonly_journal());
    sequence.send(4);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client.connection.unlock(client.client.get_readonly_journal());
    sequence.send(5);
   }
  );

  std::thread thread_1
  (
   [&server, &sequence]()
   {
    Memory_File client_file;
    Test_Client client(server, client_file);
    sequence.increment();

    sequence.wait_for(4);
    client.connection.lock(client.client.get_readonly_journal());
    sequence.wait_for(5);
    client.connection.unlock(client.client.get_readonly_journal());
   }
  );

  std::thread thread_2
  (
   [&server, &sequence]()
   {
    Memory_File client_file;
    Test_Client client(server, client_file);
    sequence.increment();

    sequence.wait_for(4);
    client.connection.lock(client.client.get_readonly_journal());
    sequence.wait_for(5);
    client.connection.unlock(client.client.get_readonly_journal());
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
  Test_Server server(false, std::chrono::seconds(0));

  Memory_File client_file;

  //
  // Fail during push
  //
  {
   Test_Client client(server, client_file);

   client.channel.set_fail_after_writing(1 << 16);

   bool caught_exception = false;

   try
   {
    client.client.transaction
    (
     [](const Readable &readable, Writable &writable)
     {
      writable.comment(std::string(1 << 18, 'x'));
     }
    );
   }
   catch (...)
   {
    caught_exception = true;
   }

   EXPECT_TRUE(caught_exception);
   EXPECT_EQ(client.client.get_journal().get_checkpoint_position(), 262189);
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
   Test_Client client(server, client_file);
   client.client.push_unlock();
   EXPECT_EQ(server.client.get_journal().get_checkpoint_position(), 262189);
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, push_timeout)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server(false, std::chrono::seconds(1));

  Memory_File client_file;
  Test_Client client(server, client_file);

  client.channel.set_fail_after_writing(1 << 16);
  client.channel.set_failure_is_timeout(true);

  bool caught_exception = false;

  try
  {
   client.client.transaction
   (
    [](const Readable &readable, Writable &writable)
    {
     writable.comment(std::string(1 << 18, 'x'));
    }
   );
  }
  catch (...)
  {
   caught_exception = true;
  }

  EXPECT_TRUE(caught_exception);

  EXPECT_EQ(client.client.get_journal().get_checkpoint_position(), 262189);
  EXPECT_EQ(server.client.get_journal().get_checkpoint_position(), 41);
  EXPECT_TRUE(server.file.get_size() > 1000);
  EXPECT_TRUE(server.file.get_size() < 262189);

  server.restart();

  client.client.push_unlock();
  EXPECT_EQ(server.client.get_journal().get_checkpoint_position(), 262189);
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, lock_timeout)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server(false, std::chrono::seconds(1));
  Memory_File client_file;
  Test_Client client(server, client_file);
  client.connection.lock(client.client.get_readonly_journal());
  std::this_thread::sleep_for(std::chrono::seconds(2));
  client.connection.unlock(client.client.get_readonly_journal());
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, ping)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server(false, std::chrono::seconds(0));
  Memory_File client_file;
  Test_Client client(server, client_file);

  {
   Channel_Lock lock(client.connection.channel);
   client.connection.ping(lock);
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, unlock_at_disconnection)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server(false, std::chrono::seconds(1));
  server.set_log(nullptr);
  Memory_File client_file;
  {
   Test_Client client(server, client_file);
   client.connection.lock(client.client.get_readonly_journal());
  }
  {
   Test_Client client(server, client_file);
   client.connection.lock(client.client.get_readonly_journal());
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, conflict)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server(false, std::chrono::seconds(0));

  Memory_File client_file_0;
  Memory_File client_file_1;

  Test_Client client_0(server, client_file_0);
  Test_Client client_1(server, client_file_1);

  client_0.client.get_writable_journal().create_table("person");
  client_0.client.get_writable_journal().default_checkpoint();

  client_1.client.get_writable_journal().create_table("person");
  client_1.client.get_writable_journal().default_checkpoint();

  client_0.client.push_unlock();
  EXPECT_ANY_THROW(client_1.client.push_unlock());
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, shared)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server(true, std::chrono::seconds(0));

  Memory_File client_file(Open_Mode::shared_write);
  Test_Client client(server, client_file);

  client.client.transaction
  (
   [](const Readable &readable, Writable &writable)
   {
    writable.create_table("person");
   }
  );

  client.connection.lock(client.client.get_readonly_journal());
  client.connection.unlock(client.client.get_readonly_journal());
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Server, double_lock)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server(false, std::chrono::seconds(0));

  Memory_File client_file;
  Test_Client client(server, client_file);

  client.connection.lock(client.client.get_readonly_journal());
  client.connection.lock(client.client.get_readonly_journal());
 }
}
