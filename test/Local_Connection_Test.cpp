#include "joedb/journal/File.h"

#ifdef JOEDB_FILE_IS_LOCKABLE
#include "joedb/concurrency/Local_Connection.h"
#include "joedb/concurrency/Interpreted_Client_Data.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/Destructor_Logger.h"
#include "joedb/String_Logger.h"

#include "gtest/gtest.h"

using namespace joedb;

static const char * const file_name = "local_connection.joedb";

/////////////////////////////////////////////////////////////////////////////
TEST(Local_Connection, simple_operation)
/////////////////////////////////////////////////////////////////////////////
{
 std::remove(file_name);

 Local_Connection connection1(file_name);
 Interpreted_Client client1(connection1);

 Local_Connection connection2(file_name);
 Interpreted_Client client2(connection2);

 client1.transaction
 (
  [](Readable &readable, Writable &writable)
  {
   writable.create_table("person");
  }
 );

 EXPECT_EQ(0, int(client2.get_database().get_tables().size()));
 client2.pull();
 EXPECT_EQ(1, int(client2.get_database().get_tables().size()));

 client2.transaction
 (
  [](Readable &readable, Writable &writable)
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
  Local_Connection connection(file_name);
  Interpreted_Client client(connection);
  FAIL() << "Expected an exception\n";
 }
 catch(const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "Checkpoint is smaller than file size. This file may contain an aborted transaction. joedb_convert can be used to fix it.");
 }
}
#endif
