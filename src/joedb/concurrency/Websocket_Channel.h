#ifndef joedb_Websocket_Channel_declared
#define joedb_Websocket_Channel_declared

#include "joedb/concurrency/Channel.h"

#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ssl.hpp>

namespace joedb
{
 class Websocket_Channel: public Channel
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
    // ssl_context.set_verify_mode(boost::asio::ssl::verify_peer);
    // peer verification is complicated
    // https://github.com/boostorg/beast/issues/2194
    // https://github.com/djarek/certify
    // https://www.reddit.com/r/cpp/comments/1ef6eje/alternatives_to_djarekcertify/
    // or manually verify from pem?

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

   size_t write_some(const char *data, size_t size) override
   {
    return ws.write(boost::asio::buffer(data, size));
   }

   size_t read_some(char *data, size_t size) override
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

#endif
