#ifndef joedb_ssh_Forward_Channel_declared
#define joedb_ssh_Forward_Channel_declared

#include "joedb/concurrency/Channel.h"
#include "joedb/ssh/Thread_Safe_Session.h"

namespace joedb
{
 namespace ssh
 {
  ///////////////////////////////////////////////////////////////////////////
  class Forward_Channel: public joedb::Channel
  ///////////////////////////////////////////////////////////////////////////
  {
   private:
    std::mutex &mutex;
    ssh_channel channel;

    std::mutex &get_mutex() override {return mutex;}

    /////////////////////////////////////////////////////////////////////////
    size_t write_some(const char *data, size_t size) override
    /////////////////////////////////////////////////////////////////////////
    {
     const int result = ssh_channel_write(channel, data, uint32_t(size));

     if (result == SSH_ERROR)
      throw joedb::Exception("Error writing to channel");

     return size_t(result);
    }

    /////////////////////////////////////////////////////////////////////////
    size_t read_some(char *data, size_t size) override
    /////////////////////////////////////////////////////////////////////////
    {
     const int result = ssh_channel_read(channel, data, uint32_t(size), 0);

     if (result == SSH_ERROR)
      throw joedb::Exception("Error reading from channel");

     if (result == 0)
      throw joedb::Exception("End of file when reading from channel");

     return size_t(result);
    }

   public:
    /////////////////////////////////////////////////////////////////////////
    Forward_Channel
    /////////////////////////////////////////////////////////////////////////
    (
     Thread_Safe_Session &session,
     const char *remote_host,
     uint16_t remote_port
    ):
     mutex(session.get_mutex())
    {
     Session_Lock lock(session);
     channel = ssh_channel_new(lock.get_ssh_session());
     check_not_null(channel);
     check_ssh_session_result
     (
      lock.get_ssh_session(),
      ssh_channel_open_forward
      (
       channel,
       remote_host,
       remote_port,
       "", // unused parameter
       0   // unused parameter
      )
     );
    }

    /////////////////////////////////////////////////////////////////////////
    ~Forward_Channel()
    /////////////////////////////////////////////////////////////////////////
    {
     try
     {
      std::unique_lock<std::mutex> lock(mutex);
      ssh_channel_free(channel);
     }
     catch(...)
     {
     }
    }
  };
 }
}

#endif
