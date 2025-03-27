#ifndef joedb_Thread_Safe_Channel_declared
#define joedb_Thread_Safe_Channel_declared

#include "joedb/concurrency/Channel.h"

#include <mutex>

namespace joedb::concurrency
{
 ////////////////////////////////////////////////////////////////////////////
 class Thread_Safe_Channel
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Channel_Lock;

  private:
   Channel &channel;
   std::mutex mutex;

  public:
   Thread_Safe_Channel(Channel &channel): channel(channel) {}
 };

 ////////////////////////////////////////////////////////////////////////////
 class Channel_Lock
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Channel &channel;
   std::unique_lock<std::mutex> lock;

  public:
   Channel_Lock(Thread_Safe_Channel &thread_safe_channel):
    channel(thread_safe_channel.channel),
    lock(thread_safe_channel.mutex)
   {
   }

   operator std::unique_lock<std::mutex> &() {return lock;}

   size_t write_some(const char *data, size_t size)
   {
    return channel.write_some(data, size);
   }

   size_t read_some(char *data, size_t size)
   {
    return channel.read_some(data, size);
   }

   void write(const char *data, size_t size)
   {
    channel.write(data, size);
   }

   void read(char *data, size_t size)
   {
    channel.read(data, size);
   }
 };
}

#endif
