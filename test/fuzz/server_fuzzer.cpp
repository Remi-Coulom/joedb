#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Writable_Journal_Client.h"
#include "joedb/concurrency/Local_Channel.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/error/Destructor_Logger.h"

/////////////////////////////////////////////////////////////////////////////
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Destructor_Logger::set_logger(nullptr);
 joedb::Memory_File file;
 joedb::Writable_Journal_Client client(file, joedb::Connection::dummy);
 joedb::Logger logger;

 const int log_level = 0;
 const int thread_count = 1;

 joedb::Server server
 (
  logger,
  log_level,
  thread_count,
  "server_fuzzer.sock",
  client,
  std::chrono::milliseconds{100}
 );

 joedb::Local_Channel channel("server_fuzzer.sock");
 channel.write((const char *)Data, Size);

 // TODO: tell server to stop after disconnection

 server.join();

 return 0;
}
