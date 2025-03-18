#ifndef joedb_Test_Network_Channel_declared
#define joedb_Test_Network_Channel_declared

#include "joedb/concurrency/Network_Channel.h"

#include <limits>
#include <thread>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Test_Network_Channel: public Network_Channel
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   size_t total_written = 0;
   size_t total_read = 0;

   bool failure_is_timeout = false;
   size_t max_write_size = std::numeric_limits<size_t>::max();
   size_t fail_after_writing = std::numeric_limits<size_t>::max();
   size_t fail_after_reading = std::numeric_limits<size_t>::max();

  protected:
   size_t write_some(const char *data, size_t size) override
   {
    if (size > max_write_size)
     size = max_write_size;

    if (total_written + size >= fail_after_writing)
    {
     const size_t write_size = fail_after_writing - total_written;

     size_t written = 0;
     if (write_size > 0)
      written = Network_Channel::write_some(data, write_size);

     if (failure_is_timeout)
     {
      std::this_thread::sleep_for(std::chrono::seconds(2));
      fail_after_writing = std::numeric_limits<size_t>::max();
     }
     else
      Network_Channel::socket.close();

     return written;
    }
    else
    {
     size_t written = Network_Channel::write_some(data, size);
     total_written += written;
     return written;
    }
   }

   size_t read_some(char *data, size_t size) override
   {
    const size_t result = Network_Channel::read_some(data, size);
    total_read += result;
    if (total_read >= fail_after_reading)
     Network_Channel::socket.close();
    return result;
   }

  public:
   Test_Network_Channel( const char *host_name, const char * port_name):
    Network_Channel(host_name, port_name)
   {
   }

   void set_failure_is_timeout(bool b)
   {
    failure_is_timeout = b;
   }

   void set_max_write_size(size_t size)
   {
    max_write_size = size;
   }

   void set_fail_after_writing(size_t size)
   {
    fail_after_writing = size;
   }

   void set_fail_after_reading(size_t size)
   {
    fail_after_reading = size;
   }
 };
}

#endif
