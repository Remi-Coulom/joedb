#include "joedb/journal/CURL_File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/File_Connection.h"
#include "joedb/concurrency/Interpreted_Client_Data.h"

#include <gtest/gtest.h>

namespace joedb::concurrency
{
 ///////////////////////////////////////////////////////////////////////////
 TEST(CURL_File, pull)
 ///////////////////////////////////////////////////////////////////////////
 {
  Memory_File memory_file;
  Writable_Interpreted_Client_Data data(memory_file);

  const bool verbose = false;

  CURL_File file
  (
   "https://github.com/Remi-Coulom/joedb/raw/master/test/endianness.joedb",
   verbose
  );

  Readonly_Journal journal(file);
  Pullonly_Journal_Connection connection(journal);

  const bool content_check = false;
  Pullonly_Client client(data, connection, content_check);
  client.pull();

  ASSERT_EQ(data.get_database().get_tables().size(), 1);
  EXPECT_EQ(data.get_database().get_tables().begin()->second, "endian");
 }
}
