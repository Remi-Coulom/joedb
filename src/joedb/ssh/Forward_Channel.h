#ifndef joedb_ssh_Forward_Channel_declared
#define joedb_ssh_Forward_Channel_declared

#include "joedb/ssh/Session.h"

namespace joedb
{
 namespace ssh
 {
  ///////////////////////////////////////////////////////////////////////////
  class Forward_Channel
  ///////////////////////////////////////////////////////////////////////////
  {
   private:
    const ssh_channel channel;

   public:
    Forward_Channel
    (
     Session &session,
     const char *remote_host,
     uint16_t remote_port
    ):
     channel(ssh_channel_new(session.get()))
    {
     check_not_null(channel);
     check_ssh_session_result
     (
      session.get(),
      ssh_channel_open_forward
      (
       channel,
       remote_host,
       remote_port,
       "localhost",
       remote_port
      )
     );
    }

    size_t write_some(const char *data, size_t size)
    {
     const int result = ssh_channel_write(channel, data, uint32_t(size));
     if (result == SSH_ERROR)
      throw joedb::Exception("Error writing to channel");
     return size_t(result);
    }

    size_t read_some(char *data, size_t size)
    {
     {
      ssh_channel channels[2] = {channel, nullptr};
      while (true)
      {
       const int result = ssh_channel_select(channels, nullptr, nullptr, nullptr);
       if (result != SSH_EINTR)
        break;
      }
     }

     const int result = ssh_channel_read(channel, data, uint32_t(size), 0);
     if (result == SSH_ERROR)
      throw joedb::Exception("Error reading from channel");

     return size_t(result);
    }

    ~Forward_Channel()
    {
     ssh_channel_free(channel);
    }
  };
 }
}

#endif
