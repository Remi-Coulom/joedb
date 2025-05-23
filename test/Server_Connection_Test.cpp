#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/protocol_version.h"
#include "joedb/concurrency/Writable_Database_Client.h"
#include "joedb/journal/Memory_File.h"
#include "gtest/gtest.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Debug_Channel: public Channel, public joedb::Memory_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   size_t write_some(const char *data, size_t size) override {return size;}
   size_t read_some(char *data, size_t size) override
   {
    read_data(data, 1);
    return 1;
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 TEST(Server_Connection, handshake)
 ////////////////////////////////////////////////////////////////////////////
 {
  Debug_Channel channel;

  for (int i = 1000; --i >= 0;)
   channel.joedb::Memory_File::write<char>('x');
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

  Buffered_File &file = channel;

  file.write<char>('j');
  file.write<char>('o');
  file.write<char>('e');
  file.write<char>('d');
  file.write<char>('b');
  file.write<int64_t>(protocol_version);
  file.write<int64_t>(1234);
  file.write<int64_t>(41);
  file.write<char>('W');
  file.write<char>('H');
  file.write<char>('F');
  file.write<int64_t>(41);
  file.write<char>('G');
  file.write<int64_t>(41);
  file.write<char>('O');
  file.set_position(0);

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
