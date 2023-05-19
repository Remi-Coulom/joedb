#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Network_Channel.h"
#include "joedb/concurrency/Interpreted_Client_Data.h"
#include "joedb/concurrency/Client.h"
#include "joedb/journal/Memory_File.h"
#include "gtest/gtest.h"

#include <joedb/journal/Writable_Journal.h>
#include <thread>
#include <fstream>

/////////////////////////////////////////////////////////////////////////////
TEST(Server, basic)
/////////////////////////////////////////////////////////////////////////////
{
 std::ofstream dummy_log;
 std::ostream * const log_stream = &dummy_log;

 joedb::Memory_File server_file;
 joedb::Writable_Journal server_journal(server_file);
 net::io_context io_context;
 joedb::Server server(server_journal, io_context, 0, 0, log_stream);
 std::thread server_thread([&io_context](){io_context.run();});

 std::ostringstream port_stream;
 port_stream << server.get_port();
 std::string port_string = port_stream.str();
 const char * const port = port_string.c_str();

 joedb::Memory_File client_file_1;
 joedb::Interpreted_Client_Data data_1(client_file_1);
 joedb::Memory_File client_file_2;
 joedb::Interpreted_Client_Data data_2(client_file_2);

 //
 // Basic operation
 //
 {
  joedb::Network_Channel channel_1("localhost", port);
  joedb::Server_Connection connection_1(data_1.get_journal(), channel_1, log_stream);
  joedb::Client client_1(data_1, connection_1);

  joedb::Network_Channel channel_2("localhost", port);
  joedb::Server_Connection connection_2(data_2.get_journal(), channel_2, log_stream);
  joedb::Client client_2(data_2, connection_2);

  client_1.pull();

  client_1.transaction
  (
   [](joedb::Readable &readable, joedb::Writable &writable)
   {
    writable.create_table("person");
   }
  );

  EXPECT_EQ(data_1.get_database().get_tables().size(), 1ULL);
  EXPECT_EQ(data_2.get_database().get_tables().size(), 0ULL);

  client_2.pull();

  EXPECT_EQ(data_2.get_database().get_tables().size(), 1ULL);

  connection_1.run_while_locked([](){});
 }

 //
 // Reconnect after disconnection, with a good non-empty database
 //
 {
  joedb::Network_Channel channel_1("localhost", port);
  joedb::Server_Connection connection_1(data_1.get_journal(), channel_1, log_stream);
  joedb::Client client_1(data_1, connection_1);
 }

 //
 // Try reconnecting with a mismatched database
 //
 {
  joedb::Memory_File file;
  joedb::Interpreted_Client_Data data(file);
  data.get_journal().create_table("city");
  data.get_journal().checkpoint(joedb::Commit_Level::no_commit);

  joedb::Network_Channel channel("localhost", port);
  joedb::Server_Connection connection(data.get_journal(), channel, log_stream);
  try
  {
   joedb::Client client(data, connection);
   FAIL() << "This should not work";
  }
  catch (const joedb::Exception &e)
  {
   EXPECT_STREQ(e.what(), "Client data does not match the server");
  }
 }

 //
 // The end
 //
 server.interrupt();
 server_thread.join();
}
