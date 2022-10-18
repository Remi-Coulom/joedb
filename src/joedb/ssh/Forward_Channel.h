#ifndef joedb_ssh_Forward_Channel_declared
#define joedb_ssh_Forward_Channel_declared

#include "joedb/concurrency/Channel.h"
#include "joedb/ssh/Session.h"

namespace joedb
{
 namespace ssh
 {
  ///////////////////////////////////////////////////////////////////////////
  class Forward_Channel: public joedb::Channel
  ///////////////////////////////////////////////////////////////////////////
  {
   private:
    ssh_channel channel;

    size_t write_some(const char *data, size_t size) final;
    size_t read_some(char *data, size_t size) final;

   public:
    Forward_Channel
    (
     Session &session,
     const char *remote_host,
     uint16_t remote_port
    );

    ~Forward_Channel();
  };
 }
}

#endif
