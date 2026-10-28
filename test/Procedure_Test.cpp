#include "joedb/error/CLog_Logger.h"
#include "joedb/rpc/Server.h"
#include "joedb/concurrency/Local_Channel.h"

#include "tutorial/Client.h"
#include "tutorial/rpc/Procedures.h"
#include "tutorial/rpc/Signatures.h"
#include "tutorial/rpc/Client.h"

#include <gtest/gtest.h>

namespace joedb
{
 class RPC_Scripted_Channel: public Channel
 {
  private:
   size_t position = 0;
  public:
   std::string incoming;
   std::string outgoing;
   template<typename T> void append(T value)
   {
    incoming.append(reinterpret_cast<const char *>(&value), sizeof(value));
   }
   void greeting(char reply = 'H')
   {
    append(reply);
    append<int64_t>(7);
   }
   size_t read_some(char *data, size_t size) override
   {
    if (position == incoming.size())
     throw Disconnection("script exhausted");
    const size_t count = std::min({size, size_t(3), incoming.size() - position});
    std::memcpy(data, incoming.data() + position, count);
    position += count;
    return count;
   }
   size_t write_some(const char *data, size_t size) override
   {
    const size_t count = std::min(size, size_t(5));
    outgoing.append(data, count);
    return count;
   }
 };

 TEST(RPC_Client, invalid_handshake)
 {
  RPC_Scripted_Channel channel;
  channel.greeting('h');
  const std::vector<rpc::Signature> signatures{{"test", std::string(Header::size, '\0')}};
  EXPECT_THROW(rpc::Client client(channel, signatures), Exception);
 }

 TEST(RPC_Client, response_size_validation)
 {
  const std::vector<rpc::Signature> signatures{{"test", std::string(Header::size, '\0')}};
  for (const char reply: {'C', 'c'})
  {
   for (const int64_t size: {int64_t(-1), int64_t(101)})
   {
    RPC_Scripted_Channel channel;
    channel.greeting();
    channel.append(reply);
    channel.append(size);
    rpc::Client client(channel, signatures, std::chrono::milliseconds(0), 100);
    Memory_File file;
    file.get_data() = signatures[0].prolog + "abc";
    const auto before = file.get_data();
    try
    {
     client.call(0, file);
     FAIL() << "Expected invalid size to be rejected";
    }
    catch (const Exception &e)
    {
     EXPECT_STREQ(e.what(), "invalid size");
    }
    EXPECT_EQ(file.get_data(), before);
   }
  }
 }

 TEST(RPC_Client, short_io_and_procedure_output)
 {
  RPC_Scripted_Channel channel;
  channel.greeting();
  channel.append<char>('C');
  channel.append<int64_t>(Header::ssize + 6);
  channel.incoming += "XYZ";
  const std::vector<rpc::Signature> signatures{{"test", std::string(Header::size, '\0')}};
  rpc::Client client(channel, signatures);
  Memory_File file;
  file.get_data() = signatures[0].prolog + "abc";
  ASSERT_NO_THROW(client.call(0, file));
  ASSERT_EQ(file.get_size(), Header::ssize + 6);
  EXPECT_EQ(file.get_data().substr(Header::size), "abcXYZ");
  int64_t checkpoint = 0;
  file.full_pread(reinterpret_cast<char *>(&checkpoint), sizeof(checkpoint), 0);
  EXPECT_EQ(checkpoint, file.get_size());
 }

 TEST(RPC_Client, ping_and_failed_disconnect)
 {
  class Failed_Disconnect_Channel: public RPC_Scripted_Channel
  {
   public:
    bool fail = false;
    size_t write_some(const char *data, size_t size) override
    {
     if (fail)
      throw Disconnection("injected disconnect failure");
     return RPC_Scripted_Channel::write_some(data, size);
    }
  } channel;
  channel.greeting();
  channel.append<char>('P');
  const std::vector<rpc::Signature> signatures{{"test", std::string(Header::size, '\0')}};
  auto client = std::make_unique<rpc::Client>(channel, signatures);
  EXPECT_NO_THROW(client->ping());
  EXPECT_EQ(channel.outgoing.back(), 'P');
  channel.fail = true;
  EXPECT_NO_THROW(client.reset());
 }

 TEST(RPC_Client, remote_error_message)
 {
  RPC_Scripted_Channel channel;
  channel.greeting();
  channel.append<char>('c');
  channel.append<int64_t>(6);
  channel.incoming += "denied";
  const std::vector<rpc::Signature> signatures{{"test", std::string(Header::size, '\0')}};
  rpc::Client client(channel, signatures);
  Memory_File file;
  file.get_data() = signatures[0].prolog;
  try
  {
   client.call(0, file);
   FAIL() << "Expected remote error";
  }
  catch (const Exception &e)
  {
   EXPECT_STREQ(e.what(), "denied");
  }
 }

 static CLog_Logger logger;
 static constexpr int log_level = 0;
 static const char * const endpoint_path = "test.rpc.sock";

 class Test_Procedure_Server
 {
  public:
   Memory_File file;
   tutorial::Client client;
   tutorial::rpc::Service service;
   tutorial::rpc::Procedures procedures;
   rpc::Server server;

  public:
   Test_Procedure_Server():
    client(file),
    service(client),
    procedures(service),
    server
    (
     logger,
     log_level,
     1,
     endpoint_path,
     tutorial::rpc::get_signatures(),
     procedures.procedures
    )
   {
    server.start();
   }

   void stop()
   {
    server.stop();
   }
 };

 TEST(Procedure, simple)
 {
  Test_Procedure_Server server;
  Local_Channel channel(endpoint_path);
  tutorial::rpc::Client rpc_client(channel);

  {
   tutorial::rpc::city::Memory_Database city;
   city.set_name("Paris");
   EXPECT_EQ(server.client.get_database().get_city_table().get_size(), 0);
   rpc_client.insert_city(city);
   EXPECT_EQ(server.client.get_database().get_city_table().get_size(), 1);
   rpc_client.delete_city(city);
   EXPECT_EQ(server.client.get_database().get_city_table().get_size(), 0);
  }

  server.stop();
 }

 TEST(Procedure, exception)
 {
  Test_Procedure_Server server;
  Local_Channel channel(endpoint_path);
  tutorial::rpc::Client rpc_client(channel);

  {
   tutorial::rpc::city::Memory_Database city;
   city.set_name("Paris");
   rpc_client.insert_city(city);
   try
   {
    rpc_client.insert_city(city);
    FAIL() << "This should have thrown";
   }
   catch (joedb::Exception &e)
   {
    EXPECT_EQ(e.what(), std::string("city already exists"));
   }
  }

  server.stop();
 }
}
