#ifndef JOEDB_PORTABLE
#include "joedb/concurrency/Local_Connection.h"
#include "joedb/journal/File.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/journal/Memory_File.h"

#include "gtest/gtest.h"

using namespace joedb;

static const char * const file_name = "local_connection.joedb";

/////////////////////////////////////////////////////////////////////////////
TEST(Local_Connection, hash_mismatch)
/////////////////////////////////////////////////////////////////////////////
{
 std::remove(file_name);
 Local_Connection<File> connection(file_name);

 {
  Memory_File client_file;
  EXPECT_ANY_THROW
  (
   Interpreted_Client client(connection, client_file)
  );
 }

 Interpreted_Client client(connection, connection.get_file());
}

/////////////////////////////////////////////////////////////////////////////
TEST(Local_Connection, simple_operation)
/////////////////////////////////////////////////////////////////////////////
{
 std::remove(file_name);

 Local_Connection<File> connection1(file_name);
 Local_Connection<File> connection2(file_name);

 Interpreted_Client client1(connection1, connection1.get_file());
 Interpreted_Client client2(connection2, connection2.get_file());

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
#endif