#ifndef joedb_Network_Channel_declared
#define joedb_Network_Channel_declared

#include "joedb/concurrency/Channel.h"
#include "joedb/concurrency/net.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Network_Channel: public Channel
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::mutex mutex;
   net::io_context io_context;
   net::ip::tcp::socket socket;

   std::mutex &get_mutex() final override {return mutex;}

   size_t write_some(const char *data, size_t size) final override;
   size_t read_some(char *data, size_t size) final override;

  public:
   Network_Channel(const char *host_name, const char *port_name);
   ~Network_Channel() override;
 };
}

#endif
