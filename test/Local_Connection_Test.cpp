#include "joedb/concurrency/Writable_Journal_Client_Data.h"
#include "joedb/concurrency/Client.h"
#include "joedb/journal/Memory_File.h"

#include "gtest/gtest.h"

namespace joedb
{
 #ifdef JOEDB_FILE_IS_LOCKABLE
 ////////////////////////////////////////////////////////////////////////////
 TEST(Local_Connection, simple_operation)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::remove(file_name);

  File file_1(file_name, Open_Mode::shared_write);
  File file_2(file_name, Open_Mode::shared_write);

  Connection connection;

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

  std::remove(file_name);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Local_Connection, size_check)
 ////////////////////////////////////////////////////////////////////////////
 {
  ::Destructor_Logger::write("Toto");
  EXPECT_EQ(::String_Logger::the_logger.get_message(), "Toto");

  std::remove(file_name);

  {
   File file(file_name, Open_Mode::create_new);
   Writable_Journal journal(file);
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
   Connection connection;
   Interpreted_Client client(connection, file);
   FAIL() << "Expected an exception\n";
  }
  catch(const Exception &e)
  {
   EXPECT_STREQ(e.what(), "Checkpoint is smaller than file size. This file may contain an aborted transaction. 'joedb_push file.joedb file fixed.joedb' can be used to truncate it.");
  }

  std::remove(file_name);
 }
 #endif

 ////////////////////////////////////////////////////////////////////////////
 TEST(Local_Connection, dummy_connection)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;

  {
   Writable_Journal_Client_Data client_data(file);
   Connection connection;
   Client client(client_data, connection);

   client.transaction([](Client_Data &data)
   {
    data.get_writable_journal().create_table("person");
   });
  }

  EXPECT_TRUE(file.get_size() > 0);
 }

 #ifdef JOEDB_FILE_IS_LOCKABLE
 ////////////////////////////////////////////////////////////////////////////
 TEST(Local_Connection, transaction_frequency)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::remove("test.joedb");

  {
   File file("test.joedb", Open_Mode::shared_write);
   Writable_Journal journal(file);

   for (int i = 10000; --i >= 0;)
   {
    journal.lock_pull();
    journal.unlock();
   }
  }

  std::remove("test.joedb");
 }
 #endif
}

