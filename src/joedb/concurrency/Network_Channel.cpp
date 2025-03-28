#include "joedb/concurrency/Network_Channel.h"

#include <asio/connect.hpp>

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////
 Network_Channel::Network_Channel
 //////////////////////////////////////////////////////////////////////////
 (
  const char *host_name,
  const char *port_name
 ):
  socket(io_context)
 {
  asio::ip::tcp::resolver resolver(io_context);
  asio::connect
  (
   socket,
   resolver.resolve
   (
    asio::ip::tcp::v4(),
    host_name,
    port_name,
    asio::ip::tcp::resolver::flags()
   )
  );
  socket.set_option(asio::ip::tcp::no_delay(true));
 }

 //////////////////////////////////////////////////////////////////////////
 size_t Network_Channel::write_some(const char *data, size_t size)
 //////////////////////////////////////////////////////////////////////////
 {
  return socket.write_some(asio::buffer(data, size));
 }

 //////////////////////////////////////////////////////////////////////////
 size_t Network_Channel::read_some(char *data, size_t size)
 //////////////////////////////////////////////////////////////////////////
 {
  return socket.read_some(asio::buffer(data, size));
 }

 //////////////////////////////////////////////////////////////////////////
 Network_Channel::~Network_Channel() = default;
 //////////////////////////////////////////////////////////////////////////
}
