#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Network_Channel.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/Writable_Journal_Client_Data.h"
#include "joedb/journal/Memory_File.h"
#include "gtest/gtest.h"

#include <thread>
#include <fstream>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 class Test_Server
 /////////////////////////////////////////////////////////////////////////////
 {
  private:
   Memory_File server_file;
   Writable_Journal_Client_Data client_data{server_file};
   Connection connection;
   Client client{client_data, connection};
   net::io_context io_context;

   Server server;
   std::thread server_thread;
   std::string port_string;

  public:
   Test_Server
   (
    bool share_client,
    std::chrono::seconds lock_timeout
   )
   :server
    {
     client,
     share_client,
     io_context,
     uint16_t(0),
     lock_timeout,
     nullptr
    },
    server_thread
    {
     [&io_context = io_context]()
     {
      io_context.run();
     }
    }
   {
    std::ostringstream port_stream;
    port_stream << server.get_port();
    port_string = port_stream.str();
   }

   const char *get_port() const
   {
    return port_string.c_str();
   }

   void set_log(std::ostream *out)
   {
    server.set_log(out);
   }

   ~Test_Server()
   {
    server.interrupt();
    server_thread.join();
   }
 };

 /////////////////////////////////////////////////////////////////////////////
 class Test_Client
 /////////////////////////////////////////////////////////////////////////////
 {
  public:
   Network_Channel channel;
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
 TEST(Server, basic)
 /////////////////////////////////////////////////////////////////////////////
 {
  Test_Server server(false, std::chrono::seconds(0));

#if 0
  server.set_log(&std::cerr);
#else
  server.set_log(nullptr);
#endif

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

#if 0
  server.set_log(&std::cerr);
#endif

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
}
