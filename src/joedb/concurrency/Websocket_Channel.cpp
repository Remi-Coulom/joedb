#include "joedb/concurrency/Websocket_Channel.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/certify/https_verification.hpp>
#include <boost/certify/extensions.hpp>

namespace joedb
{
 namespace detail
 {
  class Websocket_Channel
  {
   private:
    boost::asio::io_context io_context;
    boost::asio::ssl::context ssl_context;

    boost::beast::websocket::stream
    <
     boost::asio::ssl::stream
     <
      boost::asio::ip::tcp::socket
     >
    >
    ws;

   public:
    Websocket_Channel
    (
     const std::string &host,
     const std::string &port,
     const std::string &path
    ):
     ssl_context(boost::asio::ssl::context::tlsv12_client),
     ws(io_context, ssl_context)
    {
     ssl_context.set_verify_mode
     (
      boost::asio::ssl::context::verify_peer |
      boost::asio::ssl::context::verify_fail_if_no_peer_cert
     );

     ssl_context.set_default_verify_paths();

     boost::certify::enable_native_https_server_verification(ssl_context);
     boost::certify::set_server_hostname(ws.next_layer(), host);
     boost::certify::sni_hostname(ws.next_layer(), host);

     const auto endpoint = boost::asio::connect
     (
      boost::beast::get_lowest_layer(ws),
      boost::asio::ip::tcp::resolver(io_context).resolve(host, port)
     );

     ws.next_layer().handshake(boost::asio::ssl::stream_base::client);

     ws.binary(true);

     ws.handshake
     (
      host + ":" + std::to_string(endpoint.port()),
      path
     );
    }

    size_t write_some(const char *data, size_t size)
    {
     return ws.write(boost::asio::buffer(data, size));
    }

    size_t read_some(char *data, size_t size)
    {
     return ws.read_some(boost::asio::buffer(data, size));
    }
   
    ~Websocket_Channel()
    {
     boost::system::error_code ec;
     ws.close(boost::beast::websocket::close_code::normal, ec);
    }
  };
 }

 Websocket_Channel::Websocket_Channel
 (
  const std::string &host,
  const std::string &port,
  const std::string &path
 )
 : p(new detail::Websocket_Channel(host, port, path))
 {
 }

 size_t Websocket_Channel::write_some(const char *data, size_t size)
 {
  return p->write_some(data, size);
 }

 size_t Websocket_Channel::read_some(char *data, size_t size)
 {
  return p->read_some(data, size);
 }

 Websocket_Channel::~Websocket_Channel() = default;
}
