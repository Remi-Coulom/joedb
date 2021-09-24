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

    size_t write_some(const char *data, size_t size) override;
    size_t read_some(char *data, size_t size) override;

   public:
    Forward_Channel
    (
     Thread_Safe_Session &session,
     const char *remote_host,
     uint16_t remote_port
    );

    ~Forward_Channel();
  };
 }
}

#endif
