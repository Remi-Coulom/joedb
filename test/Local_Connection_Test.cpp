#include "joedb/concurrency/Writable_Journal_Client.h"
#include "joedb/concurrency/Writable_Database_Client.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/File.h"
#include "joedb/error/String_Logger.h"
#include "joedb/error/Destructor_Logger.h"

#include "gtest/gtest.h"

namespace joedb
{
 static const char *file_name = "test.joedb";

 ////////////////////////////////////////////////////////////////////////////
 TEST(Local_Connection, simple_operation)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!File::lockable)
   GTEST_SKIP();

  std::remove(file_name);

  File file_1(file_name, Open_Mode::shared_write);
  File file_2(file_name, Open_Mode::shared_write);

  Writable_Database_Client client1(file_1);
  Writable_Database_Client client2(file_2);

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
  if (!File::lockable)
   GTEST_SKIP();

  Destructor_Logger::write("Toto");
  EXPECT_EQ(String_Logger::the_logger.get_message(), "Toto");

  std::remove(file_name);

  {
   File file(file_name, Open_Mode::create_new);
   Writable_Journal journal(file);
   journal.timestamp(0);
   journal.flush();
  }

  EXPECT_EQ
  (
   String_Logger::the_logger.get_message(),
   "Ahead_of_checkpoint in Writable_Journal destructor"
  );

  try
  {
   File file(file_name, Open_Mode::shared_write);
   Writable_Database_Client client(file);
   FAIL() << "Expected an exception\n";
  }
  catch(const Exception &e)
  {
   EXPECT_STREQ(e.what(), "Checkpoint (41) is smaller than file size (50). This file may contain an aborted transaction. 'joedb_push file.joedb file fixed.joedb' can be used to truncate it.");
  }

  std::remove(file_name);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Local_Connection, dummy_connection)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;

  {
   Writable_Journal_Client client(file);

   client.transaction([](Writable_Journal &journal)
   {
    journal.create_table("person");
   });
  }

  EXPECT_TRUE(file.get_size() > 0);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Local_Connection, transaction_frequency)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!File::lockable)
   GTEST_SKIP();

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
}

