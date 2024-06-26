#include "joedb/concurrency/Network_Channel.h"

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
  net::ip::tcp::resolver resolver(io_context);
  net::connect
  (
   socket,
   resolver.resolve
   (
    net::ip::tcp::v4(),
    host_name,
    port_name,
    net::ip::tcp::resolver::flags()
   )
  );
  socket.set_option(net::ip::tcp::no_delay(true));
 }

 //////////////////////////////////////////////////////////////////////////
 size_t Network_Channel::write_some(const char *data, size_t size)
 //////////////////////////////////////////////////////////////////////////
 {
  return socket.write_some(net::buffer(data, size));
 }

 //////////////////////////////////////////////////////////////////////////
 size_t Network_Channel::read_some(char *data, size_t size)
 //////////////////////////////////////////////////////////////////////////
 {
  return socket.read_some(net::buffer(data, size));
 }

 //////////////////////////////////////////////////////////////////////////
 Network_Channel::~Network_Channel() = default;
 //////////////////////////////////////////////////////////////////////////
}
