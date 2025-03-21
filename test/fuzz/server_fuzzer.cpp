#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/Writable_Journal_Client_Data.h"
#include "joedb/concurrency/Network_Channel.h"
#include "joedb/journal/Memory_File.h"

#include <thread>
#include <iostream>

/////////////////////////////////////////////////////////////////////////////
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 joedb::Writable_Journal_Client_Data client_data(file);
 joedb::Connection connection;
 joedb::Client client(client_data, connection, false);

 asio::io_context io_context;

 joedb::Server server
 (
  client,
  false,
  io_context,
  1234,
  std::chrono::milliseconds{100},
  nullptr
 );

 std::thread thread([Data, Size, &io_context, &server]()
 {
  try
  {
   joedb::Network_Channel channel("localhost", "1234");
   channel.write((const char *)Data, Size);
   io_context.post([&server](){server.stop();});
  }
  catch(const std::exception &e)
  {
   std::cerr << "client exception: " << e.what() << '\n';
  }
 });

 try
 {
  io_context.run();
 }
 catch (const std::exception &e)
 {
  std::cerr << "server exception: " << e.what() << '\n';
 }

 thread.join();

 return 0;
}
