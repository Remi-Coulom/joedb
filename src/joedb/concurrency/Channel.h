#ifndef joedb_Channel_declared
#define joedb_Channel_declared

#include <mutex>
#include <stddef.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Channel
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Channel_Lock;

  private:
   virtual size_t write_some(const char *data, size_t size) = 0;
   virtual size_t read_some(char *data, size_t size) = 0;
   virtual std::mutex &get_mutex() = 0;

  public:
   virtual ~Channel() {}
 };

 ////////////////////////////////////////////////////////////////////////////
 class Channel_Lock
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Channel &channel;
   std::unique_lock<std::mutex> lock;

  public:
   Channel_Lock(Channel &channel):
    channel(channel),
    lock(channel.get_mutex())
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
    size_t n = 0;
    while (n < size)
     n += write_some(data + n, size - n);
   }

   void read(char *data, size_t size)
   {
    size_t n = 0;
    while (n < size)
     n += read_some(data + n, size - n);
   }
 };
}

#endif
