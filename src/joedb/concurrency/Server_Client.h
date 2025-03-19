#ifndef joedb_Server_Client_declared
#define joedb_Server_Client_declared

#include "joedb/concurrency/Thread_Safe_Channel.h"
#include "joedb/Posthumous_Thrower.h"
#include "joedb/journal/Buffer.h"
#include "joedb/journal/Async_Writer.h"

#include <condition_variable>
#include <thread>
#include <iosfwd>
#include <chrono>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Server_Client: Posthumous_Thrower
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Server_File;

  private:
   const std::chrono::seconds keep_alive_interval;
   std::condition_variable condition;
   void ping(Channel_Lock &lock);
   bool keep_alive_thread_must_stop;
   std::thread keep_alive_thread;
   void keep_alive();
   void connect();

  protected:
   Thread_Safe_Channel channel;
   std::ostream *log;

   Buffer<13> buffer;

   int64_t session_id;
   bool pullonly_server;
   int64_t server_checkpoint;

   void download(Async_Writer &writer, Channel_Lock &lock, int64_t size);

  public:
   Server_Client
   (
    Channel &channel,
    std::ostream *log,
    std::chrono::seconds keep_alive_interval = std::chrono::seconds{240}
   );

   static constexpr int64_t client_version = 13;
   int64_t get_session_id() const {return session_id;}
   Thread_Safe_Channel &get_channel() {return channel;}
   void ping();

   ~Server_Client();
 };
}

#endif
