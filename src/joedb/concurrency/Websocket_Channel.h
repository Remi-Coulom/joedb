#ifdef JOEDB_HAS_WEBSOCKETS
#ifndef joedb_Websocket_Channel_declared
#define joedb_Websocket_Channel_declared

#include "joedb/concurrency/Channel.h"

#include <string>
#include <memory>

namespace joedb
{
 namespace detail
 {
  class Websocket_Channel;
 }

 /// Channel to communicate with a (secure) Websocket
 class Websocket_Channel: public Channel
 {
  private:
   const std::unique_ptr<detail::Websocket_Channel> p;

  public:
   Websocket_Channel
   (
    const std::string &host,
    const std::string &port,
    const std::string &path
   );

   size_t write_some(const char *data, size_t size) override;
   size_t read_some(char *data, size_t size) override;

   ~Websocket_Channel() override;
 };
}

#endif
#endif
