#ifndef joedb_ssh_Forward_Channel_declared
#define joedb_ssh_Forward_Channel_declared

#include "joedb/concurrency/Channel.h"
#include "joedb/ssh/Session.h"

#include <chrono>

namespace joedb::ssh
{
 class Forward_Channel_Allocation
 {
  protected:
   const ssh_channel channel;

  public:
   Forward_Channel_Allocation(Session &session);
   Forward_Channel_Allocation(const Forward_Channel_Allocation &) = delete;
   Forward_Channel_Allocation &operator=(const Forward_Channel_Allocation &) = delete;
   ~Forward_Channel_Allocation();
 };

 /// @ingroup concurrency
 class Forward_Channel:
  public Forward_Channel_Allocation,
  public joedb::Channel
 {
  private:
   size_t write_some(const char *data, size_t size) override;
   size_t read_some(char *data, size_t size) override;
   std::chrono::milliseconds timeout = std::chrono::minutes{30};

  public:
   Forward_Channel
   (
    Session &session,
    const char *remote_host,
    uint16_t remote_port
   );

   Forward_Channel
   (
    Session &session,
    const char *remote_path
   );

   void set_timeout(std::chrono::milliseconds ms)
   {
    timeout = ms;
   }
 };
}

#endif
