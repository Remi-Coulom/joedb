#ifndef joedb_Server_Client_declared
#define joedb_Server_Client_declared

#include "joedb/concurrency/Thread_Safe_Channel.h"
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
   std::chrono::seconds keep_alive_interval;
   std::condition_variable condition;
   void ping(Channel_Lock &lock);
   bool keep_alive_thread_must_stop;
   std::thread keep_alive_thread;
   void keep_alive();
   void connect();
   void disconnect();

  protected:
   mutable Thread_Safe_Channel channel;
   std::ostream *log;
   bool connected;

   mutable Buffer<13> buffer;

   int64_t session_id;
   bool pullonly_server;
   int64_t server_checkpoint;

   void download(Async_Writer &writer, Channel_Lock &lock, int64_t size) const;

  public:
   Server_Client(Channel &channel);

   void set_log(std::ostream *stream)
   {
    log = stream;
   }

   void set_keep_alive_interval(std::chrono::seconds duration)
   {
    keep_alive_interval = duration;
   }

   int64_t get_session_id() const {return session_id;}
   Thread_Safe_Channel &get_channel() {return channel;}
   void ping();

   ~Server_Client();
 };
}

#endif
