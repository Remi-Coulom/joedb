#include "joedb/journal/CURL_File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/concurrency/File_Connection.h"
#include "joedb/concurrency/Writable_Database_Client.h"

#include <gtest/gtest.h>

namespace joedb
{
 ///////////////////////////////////////////////////////////////////////////
 TEST(CURL_File, pull)
 ///////////////////////////////////////////////////////////////////////////
 {
  const bool verbose = false;

  CURL_File file("https://www.joedb.org/test/v10/endianness.joedb", verbose);

  Readonly_Journal journal(file);
  Pullonly_Journal_Connection connection(journal);

  Memory_File memory_file;
  Writable_Database_Client client(memory_file, connection);

  client.pull();

  ASSERT_EQ(client.get_database().get_tables().size(), 1);
  EXPECT_EQ(client.get_database().get_tables().begin()->second, "endian");
 }
}
