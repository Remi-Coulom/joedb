#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/protocol_version.h"
#include "joedb/concurrency/Writable_Database_Client.h"
#include "joedb/journal/Memory_File.h"
#include "gtest/gtest.h"
#include "joedb/concurrency/Keep_Alive_Thread.h"
#include "joedb/concurrency/Robust_Connection.h"
#include "joedb/concurrency/Server_File.h"
#include "String_Logger.h"
#include "joedb/ui/Server_Connection_Builder.h"
#include <array>

#include <condition_variable>
#include <mutex>

namespace joedb
{
 class Ping_Probe: public Ping_Client
 {
  private:
   class Null_Channel: public Channel
   {
    size_t write_some(const char *, size_t size) override {return size;}
    size_t read_some(char *, size_t) override {return 0;}
   } raw_channel;
   Thread_Safe<Channel &> channel{raw_channel};
   std::mutex mutex;
   std::condition_variable condition;
   size_t count = 0;
   Thread_Safe<Channel &> &get_channel() override {return channel;}
   void locked_ping(Lock<Channel &> &) override
   {
    {
     std::lock_guard guard(mutex);
     ++count;
    }
    condition.notify_one();
    if (fail)
     throw Exception("ping failed");
   }

  public:
   bool fail = false;
   size_t get_count()
   {
    std::lock_guard guard(mutex);
    return count;
   }
   bool wait_for(size_t expected)
   {
    std::unique_lock lock(mutex);
    return condition.wait_for(lock, std::chrono::seconds(2), [&]() {return count >= expected;});
   }
 };

 TEST(Keep_Alive_Thread, lifecycle)
 {
  Ping_Probe probe;
  probe.ping();
  EXPECT_EQ(probe.get_count(), 1);
  Keep_Alive_Thread thread(probe, std::chrono::milliseconds(1));
  thread.start();
  thread.start();
  EXPECT_TRUE(probe.wait_for(3));
  thread.stop();
  thread.stop();
  const size_t count = probe.get_count();
  thread.start();
  EXPECT_TRUE(probe.wait_for(count + 1));
  thread.stop();
 }

 TEST(Keep_Alive_Thread, destructor_stops_running_thread)
 {
  Ping_Probe probe;
  {
   Keep_Alive_Thread thread(probe, std::chrono::milliseconds(1));
   thread.start();
   EXPECT_TRUE(probe.wait_for(1));
  }
  EXPECT_GE(probe.get_count(), 1);
 }

 TEST(Keep_Alive_Thread, disabled)
 {
  Ping_Probe probe;
  for (const int interval: {0, -1})
  {
   Keep_Alive_Thread thread(probe, std::chrono::milliseconds(interval));
   thread.start();
   thread.stop();
  }
  EXPECT_EQ(probe.get_count(), 0);
 }

 TEST(Keep_Alive_Thread, failed_ping_and_restart)
 {
  Ping_Probe probe;
  probe.fail = true;
  Keep_Alive_Thread thread(probe, std::chrono::milliseconds(1));
  thread.start();
  EXPECT_TRUE(probe.wait_for(1));
  thread.stop();
  probe.fail = false;
  const size_t count = probe.get_count();
  thread.start();
  EXPECT_TRUE(probe.wait_for(count + 1));
  thread.stop();
 }

 ////////////////////////////////////////////////////////////////////////////
 class Debug_Channel: public Channel, public Memory_File, public File_Buffer
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   size_t write_some(const char *data, size_t size) override {return size;}
   size_t read_some(char *data, size_t size) override
   {
    return File_Buffer::read_data(data, 1);
   }

  public:
   Debug_Channel(): File_Buffer(*(Memory_File *)this) {}
 };

 static void write_greeting
 (
  Debug_Channel &channel,
  int64_t version = protocol_version,
  char mode = 'W',
  int64_t checkpoint = Header::ssize
 )
 {
  channel.File_Buffer::write(Header::joedb);
  channel.File_Buffer::write(version);
  channel.File_Buffer::write<int64_t>(1);
  channel.File_Buffer::write(checkpoint);
  channel.File_Buffer::write(mode);
 }

 class Scripted_Connector: public Connector
 {
  class Reply_Channel: public Debug_Channel
  {
   size_t read_some(char *data, size_t) override
   {
    if (File_Buffer::read_data(data, 1) != 1)
     throw Exception("script exhausted");
    return 1;
   }
  };

  public:
   mutable size_t attempts = 0;
   bool fail_first = false;
   char mode = 'W';
   int64_t checkpoint = Header::ssize;
   std::string replies;
   std::unique_ptr<Channel> new_channel() const override
   {
    if (++attempts == 1 && fail_first)
     throw Exception("temporary connect failure");
    auto channel = std::make_unique<Reply_Channel>();
    write_greeting(*channel, protocol_version, mode, checkpoint);
    channel->File_Buffer::write_data(replies.data(), replies.size());
    channel->set_position(0);
    return channel;
   }
 };

 TEST(Server_Connection, builder_selects_connection_or_remote_file)
 {
  class Builder: public Server_Connection_Builder
  {
   public:
    void build_connector(Arguments &) override {connector = std::make_unique<Scripted_Connector>();}
    std::string get_connection_parameters() const override {return "scripted";}
  };
  Memory_File file;
  Logger logger;
  for (Abstract_File *local_file: {static_cast<Abstract_File *>(&file), static_cast<Abstract_File *>(nullptr)})
  {
   const std::array<const char *, 3> argv = {"test", "--keep_alive", "0"};
   Arguments arguments(argv.size(), argv.data());
   Builder builder;
   Connection *connection = builder.build(logger, arguments, local_file);
   ASSERT_TRUE(connection);
   EXPECT_EQ(dynamic_cast<Server_File *>(connection) != nullptr, local_file == nullptr);
   EXPECT_FALSE(connection->is_pullonly());
  }
 }

 TEST(Robust_Connection, retries_initial_failure_and_contains_unlock_error)
 {
  Scripted_Connector connector;
  connector.set_keep_alive_interval(std::chrono::milliseconds(0));
  connector.fail_first = true;
  connector.mode = 'R';
  connector.replies = "XM";
  String_Logger logger;
  Robust_Connection connection(connector, logger);
  EXPECT_EQ(connector.attempts, 2);
  EXPECT_TRUE(connection.is_pullonly());
  EXPECT_NO_THROW(connection.unlock());
  EXPECT_NE(logger.get_message().find("unlock() error"), std::string::npos);
  EXPECT_NO_THROW(connection.unlock());
  EXPECT_EQ(connector.attempts, 2);
 }

 TEST(Server_File, protects_remote_body_and_rejects_wrong_journal)
 {
  Scripted_Connector connector;
  connector.checkpoint = Header::ssize + 10;
  Server_File file(connector);
  EXPECT_FALSE(file.is_pullonly());
  EXPECT_THROW(file.pwrite("xx", 2, Header::ssize - 1), Disconnection);
  EXPECT_THROW(file.pwrite("x", 1, Header::ssize), Disconnection);
  Memory_File other_file;
  Writable_Journal other(other_file);
  EXPECT_THROW(file.handshake(other, Content_Check::none), Disconnection);
  EXPECT_EQ(file.get_size(), connector.checkpoint);
 }

 TEST(Server_File, pull_rejects_unpushed_tail)
 {
  Scripted_Connector connector;
  Server_File file(connector);
  file.pwrite("tail", 4, file.get_size());
  Memory_File other_file;
  Writable_Journal other(other_file);
  EXPECT_THROW(file.pull(Lock_Action::no_locking, Data_Transfer::without_data,
   other, std::chrono::milliseconds(0)), Disconnection);
  EXPECT_EQ(file.get_size(), Header::ssize + 4);
 }

 TEST(Server_Connection, rejected_greetings)
 {
  for (const int version: {0, protocol_version - 1})
  {
   Debug_Channel channel;
   write_greeting(channel, version);
   channel.set_position(0);
   EXPECT_THROW(Server_Connection connection(channel), Exception);
  }
  Debug_Channel channel;
  write_greeting(channel, protocol_version, 'X');
  channel.set_position(0);
  EXPECT_THROW(Server_Connection connection(channel), Exception);
 }

 TEST(Server_Connection, invalid_pread_sizes)
 {
  for (const int64_t until: {int64_t(-1), int64_t(5)})
  {
   Debug_Channel channel;
   write_greeting(channel);
   channel.File_Buffer::write<char>('r');
   channel.File_Buffer::write(until);
   channel.set_position(0);
   Server_Connection connection(channel);
   char data[4];
   EXPECT_THROW(connection.pread(data, sizeof(data), 0), Disconnection);
  }
 }

 TEST(Server_Connection, unexpected_pread_reply)
 {
  Debug_Channel channel;
  write_greeting(channel);
  channel.File_Buffer::write<char>('X');
  channel.File_Buffer::write<int64_t>(3);
  channel.File_Buffer::write_data("abc", 3);
  channel.set_position(0);
  Server_Connection connection(channel);
  char data[3];
  EXPECT_THROW(connection.pread(data, sizeof(data), 0), Disconnection);
 }

 TEST(Server_Connection, push_rejects_truncated_source)
 {
  Memory_File file;
  Writable_Journal journal(file);
  journal.comment("checkpointed data");
  journal.soft_checkpoint();
  const int64_t checkpoint = journal.get_checkpoint();
  file.get_data().resize(Header::size);
  Debug_Channel channel;
  write_greeting(channel);
  channel.set_position(0);
  Server_Connection connection(channel);
  EXPECT_THROW(connection.push(journal, Header::ssize, checkpoint,
   Unlock_Action::unlock_after), Disconnection);
 }

 TEST(Server_Connection, rejected_pull_push_and_unlock)
 {
  Memory_File file;
  Writable_Journal journal(file);
  for (const char reply: {'R', 'X'})
  {
   Debug_Channel channel;
   write_greeting(channel);
   channel.File_Buffer::write(reply);
   channel.File_Buffer::write<int64_t>(Header::ssize);
   channel.set_position(0);
   Server_Connection connection(channel);
   EXPECT_THROW(connection.pull(Lock_Action::no_locking, Data_Transfer::with_data, journal), Disconnection);
  }
  for (const char reply: {'R', 'C', 't', 'X'})
  {
   SCOPED_TRACE(reply);
   Debug_Channel channel;
   write_greeting(channel);
   channel.File_Buffer::write(reply);
   channel.set_position(0);
   Server_Connection connection(channel);
   EXPECT_THROW(connection.push(journal, Header::ssize, Header::ssize,
    Unlock_Action::unlock_after), Exception);
  }
  Debug_Channel channel;
  write_greeting(channel);
  channel.File_Buffer::write<char>('X');
  channel.set_position(0);
  Server_Connection connection(channel);
  EXPECT_THROW(connection.unlock(), Disconnection);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server_Connection, handshake)
 ////////////////////////////////////////////////////////////////////////////
 {
  Debug_Channel channel;

  for (int i = 1000; --i >= 0;)
   channel.File_Buffer::write<char>('x');
  channel.set_position(0);

  try
  {
   Server_Connection connection(channel);

   ADD_FAILURE() << "Should have thrown";
  }
  catch (const Exception &e)
  {
   EXPECT_STREQ(e.what(), "Did not receive \"joedb\" from server");
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server_Connection, session)
 ////////////////////////////////////////////////////////////////////////////
 {
  Debug_Channel channel;

  channel.File_Buffer::write<char>('j');
  channel.File_Buffer::write<char>('o');
  channel.File_Buffer::write<char>('e');
  channel.File_Buffer::write<char>('d');
  channel.File_Buffer::write<char>('b');
  channel.File_Buffer::write<int64_t>(protocol_version);
  channel.File_Buffer::write<int64_t>(1234);
  channel.File_Buffer::write<int64_t>(41);
  channel.File_Buffer::write<char>('W');
  channel.File_Buffer::write<char>('F');
  channel.File_Buffer::write<int64_t>(41);
  channel.File_Buffer::write<char>('G');
  channel.File_Buffer::write<int64_t>(41);
  channel.File_Buffer::write<char>('O');
  channel.set_position(0);

  {
   Server_Connection connection(channel);
   Memory_File client_file;
   Writable_Database_Client client(client_file, connection);

   EXPECT_EQ(connection.get_session_id(), 1234);

   client.pull();
   client.transaction([](Readable &, Writable &){});
  }
 }
}
