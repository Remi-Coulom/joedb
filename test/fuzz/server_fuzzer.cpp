#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Writable_Journal_Client.h"
#include "joedb/concurrency/Local_Channel.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/error/Destructor_Logger.h"

#include <thread>

/////////////////////////////////////////////////////////////////////////////
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Destructor_Logger::set_logger(nullptr);
 joedb::Memory_File file;
 joedb::Connection connection;
 joedb::Writable_Journal_Client client(file, connection);

 asio::io_context io_context;

 joedb::Server server
 (
  client,
  io_context,
  "server_fuzzer.sock",
  std::chrono::milliseconds{100},
  nullptr
 );

 std::thread thread([Data, Size, &io_context, &server]()
 {
  joedb::Local_Channel channel("server_fuzzer.sock");
  channel.write((const char *)Data, Size);
  io_context.post([&server](){server.stop_after_sessions();});
 });

 try
 {
  io_context.run();
 }
 catch (const joedb::Exception &)
 {
 }

 thread.join();

 return 0;
}
