#ifdef JOEDB_HAS_SSH

#include "joedb/ssh/Session.h"

#include "gtest/gtest.h"

#ifdef JOEDB_HAS_ASIO
#include "Test_TCP_Drop_Server.h"
#include "Test_SSH_Server.h"
#include "joedb/ssh/Forward_Channel.h"
#include "joedb/ssh/SFTP.h"
#include "joedb/journal/SFTP_File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/ui/SSH_Connection_Builder.h"
#include <array>
#endif

namespace joedb
{
#ifdef JOEDB_HAS_ASIO
 TEST(ssh, sftp_random_access_short_reads_and_copy)
 {
  Test_SSH_Server server(true);
  Test_SSH_Key key;
  ssh::Session session("joedb-test", "127.0.0.1", server.port(), 0, key.get_text(),
   nullptr, server.get_known_hosts_file());
  ssh::SFTP sftp(session);
  SFTP_File file(sftp, "payload");
  char data[6] = {};
  EXPECT_EQ(file.pread(data, 3, 2), 3);
  EXPECT_EQ(std::string_view(data, 3), "234");
  EXPECT_EQ(file.pread(data, 3, 5), 3);
  EXPECT_EQ(std::string_view(data, 3), "567");
  file.full_pread(data, sizeof(data), 0);
  EXPECT_EQ(std::string_view(data, sizeof(data)), "012345");
  EXPECT_EQ(file.pread(data, sizeof(data), 10), 0);
  Memory_File copied;
  ASSERT_NO_THROW(file.copy_to(copied));
  EXPECT_EQ(copied.get_data(), "0123456789");
 }

 TEST(ssh, sftp_open_and_read_errors)
 {
  Test_SSH_Server server(true);
  Test_SSH_Key key;
  ssh::Session session("joedb-test", "127.0.0.1", server.port(), 0, key.get_text(),
   nullptr, server.get_known_hosts_file());
  ssh::SFTP sftp(session);
  EXPECT_THROW(SFTP_File(sftp, "missing"), Exception);
  SFTP_File file(sftp, "broken");
  char data = 0;
  EXPECT_THROW(file.pread(&data, 1, 0), Exception);
 }

 TEST(ssh, builder_parses_key_and_endpoint)
 {
  Test_SSH_Server server;
  Test_SSH_Key key;
  const std::string port = std::to_string(server.port());
  const std::array<const char *, 8> argv =
   {"test", "--port", port.c_str(), "--key", key.get_text(), "joedb-test", "127.0.0.1", "/rejected.sock"};
  Arguments arguments(argv.size(), argv.data());
  class Builder: public SSH_Connection_Builder
  {
   public:
    using Server_Connection_Builder::connector;
  } builder;
  builder.build_connector(arguments);
  ASSERT_FALSE(arguments.missing());
  ASSERT_TRUE(builder.connector);
  EXPECT_THROW(builder.connector->new_channel(), Exception);
 }

 TEST(ssh, builder_rejects_missing_endpoint_arguments)
 {
  const std::array<const char *, 4> argv = {"test", "joedb-test", "127.0.0.1", "/socket"};
  Logger logger;
  for (size_t count = 1; count < argv.size(); ++count)
  {
   Arguments arguments(int(count), argv.data());
   SSH_Connection_Builder builder;
   EXPECT_EQ(builder.build(logger, arguments, nullptr), nullptr);
   EXPECT_TRUE(arguments.missing());
  }
 }

 TEST(ssh, local_public_key_authentication_and_forwarded_echo)
 {
  Test_SSH_Server server;
  Test_SSH_Key key;
  ssh::Session session("joedb-test", "127.0.0.1", server.port(), 0, key.get_text(),
   nullptr, server.get_known_hosts_file());
  ssh::Forward_Channel forwarded(session, "echo.invalid", 1234);
  forwarded.set_timeout(std::chrono::seconds(2));
  Channel &channel = forwarded;
  const std::string message("a\0b", 3);
  channel.write(message.data(), message.size());
  std::string echo(message.size(), '?');
  channel.read(echo.data(), echo.size());
  EXPECT_EQ(echo, message);
  char data = 0;
  EXPECT_THROW(channel.read_some(&data, 1), Exception);
 }

 TEST(ssh, rejects_unknown_host_key)
 {
  Test_SSH_Server server;
  Test_SSH_Key key;
  EXPECT_THROW(ssh::Session("joedb-test", "127.0.0.1", server.port(), 0, key.get_text()), Disconnection);
 }

 TEST(ssh, rejected_forward_and_sftp_requests)
 {
  Test_SSH_Server server;
  Test_SSH_Key key;
  ssh::Session session("joedb-test", "127.0.0.1", server.port(), 0, key.get_text(),
   nullptr, server.get_known_hosts_file());
  EXPECT_THROW(ssh::Forward_Channel(session, "/rejected.sock"), Exception);
  EXPECT_THROW(ssh::SFTP{session}, Exception);
 }

 TEST(ssh, peer_disconnects_during_handshake)
 {
  Test_TCP_Drop_Server server;
  EXPECT_THROW(ssh::Session("joedb-test", "127.0.0.1", server.port(), 0), Exception);
  EXPECT_TRUE(server.accepted_connection());
 }
#endif
 static constexpr const char *b64_key =
R"RRR(-----BEGIN OPENSSH PRIVATE KEY-----
b3BlbnNzaC1rZXktdjEAAAAACmFlczI1Ni1jdHIAAAAGYmNyeXB0AAAAGAAAABBADubSlG
1EmKeHf40MxSvVAAAAGAAAAAEAAAAzAAAAC3NzaC1lZDI1NTE5AAAAIDEcGEaP6uBECIzA
mko9gVD9DFRjC5kkRpZAvsgGPYFOAAAAkCTRYY8/xyKDf/439HjU7cAGaHT+J8ilIB4fnz
rQY/CG8yT5qRLqeo6yUnlSI+N2arr59OYsY7/8nKTlOnUcrFz/8umS0EKOUg7EQg9yYVaG
QOyBSpFoucn5gIuppkfB4/5FGlM5whgkW+NhtGxzXKR+ZV/HL0OPWEtcibmUu0XDoOky1b
2DG0OJUmvv8AfHvw==
-----END OPENSSH PRIVATE KEY-----)RRR";

 ////////////////////////////////////////////////////////////////////////////
 TEST(ssh, Imported_Key_OK)
 ////////////////////////////////////////////////////////////////////////////
 {
  const ssh::Imported_Key key(b64_key, "joedb");
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(ssh, Imported_Key_bad_passphrase)
 ////////////////////////////////////////////////////////////////////////////
 {
  EXPECT_ANY_THROW
  (
   const ssh::Imported_Key key(b64_key, "y");
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(ssh, Imported_Key_bad_key)
 ////////////////////////////////////////////////////////////////////////////
 {
  EXPECT_ANY_THROW
  (
   const ssh::Imported_Key key("x", "y");
  );
 }
}

#endif
