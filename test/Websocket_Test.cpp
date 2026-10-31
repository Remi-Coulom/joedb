#ifdef JOEDB_HAS_WEBSOCKETS
#include "joedb/concurrency/Websocket_Channel.h"
#include "joedb/ui/Websocket_Connection_Builder.h"

#include "gtest/gtest.h"

#include <array>
#include "Test_TCP_Drop_Server.h"

#if defined(__unix__) || defined(__APPLE__)
#include "Test_TLS_Websocket_Server.h"
#endif

namespace joedb
{
#if defined(__unix__) || defined(__APPLE__)
 TEST(Websocket, builder_parses_endpoint_and_connects)
 {
  const Test_TLS_Certificate certificate;
  const Test_TLS_Trust trust(certificate.certificate);
  Test_TLS_Websocket_Server server(certificate);
  const std::string port = server.port();
  const std::array<const char *, 4> argv = {"test", "localhost", port.c_str(), "/echo"};
  Arguments arguments(argv.size(), argv.data());
  class Builder: public Websocket_Connection_Builder
  {
   public:
    using Server_Connection_Builder::connector;
  } builder;
  builder.build_connector(arguments);
  ASSERT_FALSE(arguments.missing());
  ASSERT_TRUE(builder.connector);
  auto channel = builder.connector->new_channel();
  channel->write("test", 4);
  std::string echo(4, '?');
  channel->read(echo.data(), echo.size());
  EXPECT_EQ(echo, "test");
 }

 TEST(Websocket, builder_rejects_missing_endpoint_arguments)
 {
  const std::array<const char *, 4> argv = {"test", "localhost", "1234", "/echo"};
  Logger logger;
  for (size_t count = 1; count < argv.size(); ++count)
  {
   Arguments arguments(int(count), argv.data());
   Websocket_Connection_Builder builder;
   EXPECT_EQ(builder.build(logger, arguments, nullptr), nullptr);
   EXPECT_TRUE(arguments.missing());
  }
 }

 TEST(Websocket, peer_closes_after_echo)
 {
  const Test_TLS_Certificate certificate;
  const Test_TLS_Trust trust(certificate.certificate);
  Test_TLS_Websocket_Server server(certificate, true);
  Websocket_Channel channel("localhost", server.port(), "/");
  channel.write("echo", 4);
  std::string echo(4, '?');
  channel.read(echo.data(), echo.size());
  EXPECT_EQ(echo, "echo");
  char byte = 0;
  EXPECT_THROW(channel.read_some(&byte, 1), std::exception);
 }

 TEST(Websocket, rejects_untrusted_local_certificate)
 {
  const Test_TLS_Certificate certificate;
  Test_TLS_Websocket_Server server(certificate);
  EXPECT_THROW(Websocket_Channel("localhost", server.port(), "/"), std::exception);
 }

 TEST(Websocket, rejects_hostname_mismatch)
 {
  const Test_TLS_Certificate certificate;
  const Test_TLS_Trust trust(certificate.certificate);
  Test_TLS_Websocket_Server server(certificate);
  EXPECT_THROW(Websocket_Channel("127.0.0.1", server.port(), "/"), std::exception);
 }

 TEST(Websocket, local_verified_tls_echo)
 {
  const Test_TLS_Certificate certificate;
  const Test_TLS_Trust trust(certificate.certificate);
  Test_TLS_Websocket_Server server(certificate);
  Websocket_Channel channel("localhost", server.port(), "/echo");
  for (const auto &message: {std::string("a\0b", 3), std::string(20000, 'x')})
  {
   ASSERT_EQ(channel.write_some(message.data(), message.size()), message.size());
   std::string echo(message.size(), '?');
   channel.read(echo.data(), echo.size());
   EXPECT_EQ(echo, message);
  }
 }
#endif
 TEST(Websocket, peer_disconnects_during_tls_handshake)
 {
  Test_TCP_Drop_Server server;
  EXPECT_THROW(Websocket_Channel("127.0.0.1", std::to_string(server.port()), "/"), std::exception);
  EXPECT_TRUE(server.accepted_connection());
 }
 TEST(Websocket, basic)
 {
  GTEST_SKIP();
  Websocket_Channel channel("localhost", "8080", "/");
  std::string_view message("Hello");

  for (int count = 10; --count >= 0;)
  {
   channel.write(message.data(), message.size());
   std::array<char, 128> buffer;
   const size_t n = channel.read_some(buffer.data(), buffer.size());
   std::cerr << count << ' ' << std::string_view(buffer.data(), n) << '\n';
  }
 }
}
#endif
