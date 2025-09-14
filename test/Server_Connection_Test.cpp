#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/protocol_version.h"
#include "joedb/concurrency/Writable_Database_Client.h"
#include "joedb/journal/Memory_File.h"
#include "gtest/gtest.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Debug_Channel: public Channel, public Memory_File, public File_Buffer
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   size_t write_some(const char *data, size_t size) override {return size;}
   size_t read_some(char *data, size_t size) override
   {
    read_data(data, 1);
    return 1;
   }

  public:
   Debug_Channel(): File_Buffer(*(Memory_File *)this) {}
 };

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
  channel.File_Buffer::write<char>('H');
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
