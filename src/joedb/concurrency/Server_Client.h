#ifndef joedb_Server_Client_declared
#define joedb_Server_Client_declared

#include "joedb/concurrency/Channel.h"
#include "joedb/Thread_Safe.h"
#include "joedb/journal/Buffer.h"
#include "joedb/journal/Async_Writer.h"

#include <condition_variable>
#include <thread>
#include <iosfwd>
#include <chrono>

namespace joedb
{
 /// @ingroup concurrency
 class Server_Client
 {
  friend class Server_File;

  private:
   std::chrono::milliseconds keep_alive_interval;
   std::condition_variable condition;
   void ping(Lock<Channel&> &lock);
   bool keep_alive_thread_must_stop;
   std::thread keep_alive_thread;
   void keep_alive();
   void connect();
   void disconnect() noexcept;

  protected:
   mutable Thread_Safe<Channel&> channel;
   std::ostream *log;
   bool connected;

   mutable Buffer<13> buffer;

   int64_t session_id;
   bool pullonly_server;
   int64_t server_checkpoint;

   void download
   (
    Async_Writer &writer,
    Lock<Channel&> &lock,
    int64_t size
   ) const;

  public:
   ///
   /// @param keep_alive_interval: a background thread will send a ping
   /// to the server at this interval. Setting this parameter to zero disables
   /// the keep-alive background thread.
   ///
   Server_Client
   (
    Channel &channel,
    std::ostream *log = nullptr,
    std::chrono::milliseconds keep_alive_interval = std::chrono::seconds(0)
   );

   int64_t get_session_id() const {return session_id;}
   Thread_Safe<Channel&> &get_channel() {return channel;}
   void ping();

   ~Server_Client();
 };
}

#endif
