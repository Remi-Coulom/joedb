#include "joedb/journal/File.h"
#include "joedb/concurrency/Journal_Client_Data.h"

#include "joedb/concurrency/Local_Connection.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/Destructor_Logger.h"
#include "joedb/String_Logger.h"

#include "gtest/gtest.h"

using namespace joedb;

static const char * const file_name = "local_connection.joedb";

#ifdef JOEDB_FILE_IS_LOCKABLE
/////////////////////////////////////////////////////////////////////////////
TEST(Local_Connection, simple_operation)
/////////////////////////////////////////////////////////////////////////////
{
 std::remove(file_name);

 File file_1(file_name, Open_Mode::shared_write);
 File file_2(file_name, Open_Mode::shared_write);

 Local_Connection connection;

 Interpreted_Client client1(connection, file_1);
 Interpreted_Client client2(connection, file_2);

 client1.transaction
 (
  [](const Readable &readable, Writable &writable)
  {
   writable.create_table("person");
  }
 );

 EXPECT_EQ(0, int(client2.get_database().get_tables().size()));
 client2.pull();
 EXPECT_EQ(1, int(client2.get_database().get_tables().size()));

 client2.transaction
 (
  [](const Readable &readable, Writable &writable)
  {
   writable.create_table("city");
  }
 );

 EXPECT_EQ(1, int(client1.get_database().get_tables().size()));
 client1.pull();
 EXPECT_EQ(2, int(client1.get_database().get_tables().size()));
}

/////////////////////////////////////////////////////////////////////////////
TEST(Local_Connection, size_check)
/////////////////////////////////////////////////////////////////////////////
{
 ::Destructor_Logger::write("Toto");
 EXPECT_EQ(::String_Logger::the_logger.get_message(), "Toto");

 std::remove(file_name);

 {
  joedb::File file(file_name, joedb::Open_Mode::create_new);
  joedb::Writable_Journal journal(file);
  journal.timestamp(0);
  journal.flush();
 }

 EXPECT_EQ
 (
  ::String_Logger::the_logger.get_message(),
  "Ahead_of_checkpoint in Writable_Journal destructor"
 );

 try
 {
  File file(file_name, Open_Mode::shared_write);
  Local_Connection connection;
  Interpreted_Client client(connection, file);
  FAIL() << "Expected an exception\n";
 }
 catch(const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "Checkpoint is smaller than file size. This file may contain an aborted transaction. joedb_convert can be used to fix it.");
 }
}
#endif

/////////////////////////////////////////////////////////////////////////////
TEST(Local_Connection, must_be_shared)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 joedb::Local_Connection connection;
 joedb::Journal_Client_Data data(file);
 joedb::Client client(data, connection);
 EXPECT_ANY_THROW(client.pull());
 EXPECT_ANY_THROW(client.transaction([](){}));
 EXPECT_ANY_THROW(client.push_unlock());
}

/////////////////////////////////////////////////////////////////////////////
TEST(Local_Connection, dummy_connection)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;

 {
  joedb::Journal_Client_Data data(file);
  joedb::Connection connection;
  joedb::Client client(data, connection);

  client.transaction([&data]()
  {
   data.get_journal().create_table("person");
  });
 }

 EXPECT_TRUE(file.get_size() > 0);
}
