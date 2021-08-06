#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/journal/Memory_File.h"
#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
class Debug_Channel: public joedb::Channel, public joedb::Memory_File
/////////////////////////////////////////////////////////////////////////////
{
 private:
  std::mutex mutex;

  size_t write_some(const char *data, size_t size) override {return size;}
  size_t read_some(char *data, size_t size) override
  {
   read_data(data, 1);
   return 1;
  }
  std::mutex &get_mutex() override {return mutex;}

 public:
};

/////////////////////////////////////////////////////////////////////////////
TEST(Server_Connection, basic)
/////////////////////////////////////////////////////////////////////////////
{
 Debug_Channel channel;

 channel.write<char>('x');
 channel.set_position(0);

 try
 {
  joedb::Server_Connection connection(channel);
  FAIL() << "Should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "Did not receive \"joedb\" from server");
 }

 channel.set_position(0);
 channel.write<char>('j');
 channel.write<char>('o');
 channel.write<char>('e');
 channel.write<char>('d');
 channel.write<char>('b');
 channel.write<int64_t>(0);
 channel.write<int64_t>(1);
 channel.set_position(0);

 try
 {
  joedb::Server_Connection connection(channel);
  FAIL() << "Should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "Client version rejected by server");
 }

 channel.set_position(0);
 channel.write<char>('j');
 channel.write<char>('o');
 channel.write<char>('e');
 channel.write<char>('d');
 channel.write<char>('b');
 channel.write<int64_t>(3);
 channel.write<int64_t>(1234);
 channel.write<char>('l');
 channel.write<char>('u');
 channel.write<char>('P');
 channel.write<int64_t>(41);
 channel.write<int64_t>(0);
 channel.write<char>('L');
 channel.write<int64_t>(41);
 channel.write<int64_t>(0);
 channel.write<char>('U');
 channel.set_position(0);

 {
  joedb::Server_Connection connection(channel);
  EXPECT_EQ(connection.get_session_id(), 1234);

  {
   joedb::Mutex_Lock lock (connection);
  }

  joedb::Memory_File client_file;
  joedb::Interpreted_Client client(connection, client_file);

  client.pull();

  client.write_transaction([](joedb::Readable_Writable &db){});
 }
}
