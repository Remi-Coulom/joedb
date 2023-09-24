#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/journal/Memory_File.h"
#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
class Debug_Channel: public joedb::Channel, public joedb::Memory_File
/////////////////////////////////////////////////////////////////////////////
{
 private:
  size_t write_some(const char *data, size_t size) override {return size;}
  size_t read_some(char *data, size_t size) override
  {
   read_data(data, 1);
   return 1;
  }
};

/////////////////////////////////////////////////////////////////////////////
TEST(Server_Connection, handshake)
/////////////////////////////////////////////////////////////////////////////
{
 std::ostream * const log = nullptr;
 Debug_Channel channel;

 channel.joedb::Memory_File::write<char>('x');
 channel.set_position(0);

 joedb::Server_Connection connection(channel, log);

 try
 {
  joedb::Memory_File client_file;
  joedb::Interpreted_Client client(connection, client_file);

  ADD_FAILURE() << "Should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "Did not receive \"joedb\" from server");
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Server_Connection, session)
/////////////////////////////////////////////////////////////////////////////
{
 std::ostream * const log = nullptr;
 Debug_Channel channel;

 joedb::Generic_File &file = channel;

 file.write<char>('j');
 file.write<char>('o');
 file.write<char>('e');
 file.write<char>('d');
 file.write<char>('b');
 file.write<int64_t>(7);
 file.write<int64_t>(1234);
 file.write<int64_t>(41);
 file.write<char>('H');
 file.write<char>('P');
 file.write<int64_t>(41);
 file.write<int64_t>(0);
 file.write<char>('L');
 file.write<int64_t>(41);
 file.write<int64_t>(0);
 file.write<char>('u');
 file.set_position(0);

 {
  joedb::Memory_File client_file;
  joedb::Interpreted_Client_Data data(client_file);
  joedb::Server_Connection connection(channel, log);
  joedb::Client client(data, connection);

  EXPECT_EQ(connection.get_session_id(), 1234);

  client.pull();

  // NOLINTNEXTLINE(readability-named-parameter)
  client.transaction([](joedb::Client_Data &){});
 }
}
