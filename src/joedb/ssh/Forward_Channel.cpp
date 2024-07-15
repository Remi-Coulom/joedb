#include "joedb/ssh/Forward_Channel.h"

namespace joedb
{
 namespace ssh
 {
  ///////////////////////////////////////////////////////////////////////////
  Forward_Channel_Allocation::Forward_Channel_Allocation(Session &session):
  ///////////////////////////////////////////////////////////////////////////
   channel(ssh_channel_new(session.get()))
  {
   check_not_null(channel);
  }

  ///////////////////////////////////////////////////////////////////////////
  Forward_Channel_Allocation::~Forward_Channel_Allocation()
  ///////////////////////////////////////////////////////////////////////////
  {
   ssh_channel_free(channel);
  }

  ///////////////////////////////////////////////////////////////////////////
  size_t Forward_Channel::write_some(const char *data, size_t size)
  ///////////////////////////////////////////////////////////////////////////
  {
   const int result = ssh_channel_write(channel, data, uint32_t(size));

   if (result == SSH_ERROR)
    throw joedb::Exception("Error writing to channel");

   return size_t(result);
  }

  ///////////////////////////////////////////////////////////////////////////
  size_t Forward_Channel::read_some(char *data, size_t size)
  ///////////////////////////////////////////////////////////////////////////
  {
   const int result = ssh_channel_read(channel, data, uint32_t(size), 0);

   if (result == SSH_ERROR)
    throw joedb::Exception("Error reading from channel");

   if (result == 0)
    throw joedb::Exception("End of file when reading from channel");

   return size_t(result);
  }

  ///////////////////////////////////////////////////////////////////////////
  Forward_Channel::Forward_Channel
  ///////////////////////////////////////////////////////////////////////////
  (
   Session &session,
   const char *remote_host,
   uint16_t remote_port
  ):
   Forward_Channel_Allocation(session)
  {
   session.check_result
   (
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
 }
}
