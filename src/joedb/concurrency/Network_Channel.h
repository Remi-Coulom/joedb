#ifndef joedb_Network_Channel_declared
#define joedb_Network_Channel_declared

#include "joedb/concurrency/Channel.h"

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>

namespace joedb
{
 /// @ingroup concurrency
 class Network_Channel: public Channel
 {
  protected:
   asio::io_context io_context;
   asio::ip::tcp::socket socket;

   size_t write_some(const char *data, size_t size) override;
   size_t read_some(char *data, size_t size) override;

  public:
   Network_Channel(std::string_view host, std::string_view service);
   ~Network_Channel() override;
 };
}

#endif
