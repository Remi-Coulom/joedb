#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Network_Channel.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/journal/Memory_File.h"
#include "gtest/gtest.h"

#include <thread>

/////////////////////////////////////////////////////////////////////////////
TEST(Server, basic)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File server_file;
 joedb::Writable_Journal server_journal(server_file);
 net::io_context io_context;
 joedb::Server server(server_journal, io_context, 0, 0);
 std::thread server_thread([&io_context](){io_context.run();});

 std::ostringstream port_stream;
 port_stream << server.get_port();
 std::string port_string = port_stream.str();
 const char * const port = port_string.c_str();

 joedb::Network_Channel channel_1("localhost", port);
 joedb::Server_Connection connection_1(channel_1);
 joedb::Memory_File client_file_1;
 joedb::Interpreted_Client client_1(connection_1, client_file_1);

 joedb::Network_Channel channel_2("localhost", port);
 joedb::Server_Connection connection_2(channel_2);
 joedb::Memory_File client_file_2;
 joedb::Interpreted_Client client_2(connection_2, client_file_2);

 client_1.pull();

 client_1.write_transaction([](joedb::Readable_Writable &db)
 {
  db.create_table("person");
 });

 EXPECT_EQ(client_1.get_database().get_tables().size(), 1ULL);
 EXPECT_EQ(client_2.get_database().get_tables().size(), 0ULL);

 client_2.pull();

 EXPECT_EQ(client_2.get_database().get_tables().size(), 1ULL);

 server.interrupt();
 server_thread.join();
}
