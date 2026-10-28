#include "joedb/journal/CURL_File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/concurrency/File_Connection.h"
#include "joedb/concurrency/Writable_Database_Client.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#ifdef JOEDB_HAS_ASIO
#include "Test_HTTP_Range_Server.h"
#endif

namespace joedb
{
#ifdef JOEDB_HAS_ASIO
 TEST(CURL_HTTP_File, bounded_ranges)
 {
  Test_HTTP_Range_Server server;
  CURL_File source(server.url().c_str(), false);
  const Abstract_File &file = source;
  char data[4] = {};
  ASSERT_EQ(file.pread(data, 4, 3), 4);
  EXPECT_EQ(std::string_view(data, 4), "3456");
  Memory_File copied;
  ASSERT_NO_THROW(file.copy_to(copied, 3, 4));
  EXPECT_EQ(copied.get_data(), std::string(3, '\0') + "3456");
 }

 TEST(CURL_HTTP_File, read_after_end_returns_zero)
 {
  Test_HTTP_Range_Server server;
  CURL_File source(server.url().c_str(), false);
  const Abstract_File &file = source;
  char data = '?';
  size_t count = 1;
  ASSERT_NO_THROW(count = file.pread(&data, 1, 20));
  EXPECT_EQ(count, 0);
  EXPECT_EQ(data, '?');
 }

 TEST(CURL_HTTP_File, copies_whole_file_of_unknown_size)
 {
  Test_HTTP_Range_Server server;
  CURL_File source(server.url().c_str(), false);
  const Abstract_File &file = source;
  ASSERT_EQ(file.get_size(), -1);
  Memory_File copied;
  ASSERT_NO_THROW(file.copy_to(copied));
  EXPECT_EQ(copied.get_data(), "0123456789");
 }
#endif

 class Local_CURL_File_Test: public testing::Test
 {
  protected:
   const char *file_name = "curl_range_test.tmp";
   std::string url;
   void SetUp() override
   {
    std::ofstream out(file_name, std::ios::binary);
    out << "abcdefghij";
    const std::string path = std::filesystem::absolute(file_name).generic_string();
    url = std::string("file://") + (path.front() == '/' ? "" : "/") + path;
   }
   void TearDown() override {std::remove(file_name);}
 };

 TEST_F(Local_CURL_File_Test, read_and_copy_ranges)
 {
  CURL_File curl(url.c_str(), false);
  Abstract_File &file = curl;
  std::string data(4, '?');
  ASSERT_NO_THROW(file.full_pread(data.data(), data.size(), 3));
  EXPECT_EQ(data, "defg");
  Memory_File copy;
  copy.get_data() = "xxxxxxxxxx";
  EXPECT_NO_THROW(file.copy_to(copy, 2, 4));
  EXPECT_EQ(copy.get_data(), "xxcdefxxxx");
 }

 TEST_F(Local_CURL_File_Test, copy_entire_file)
 {
  CURL_File curl(url.c_str(), false);
  Abstract_File &file = curl;
  Memory_File copy;
  EXPECT_NO_THROW(file.copy_to(copy));
  EXPECT_EQ(copy.get_data(), "abcdefghij");
 }

 TEST_F(Local_CURL_File_Test, zero_length_read)
 {
  CURL_File curl(url.c_str(), false);
  Abstract_File &file = curl;
  char data = '?';
  for (const int64_t offset: {0, 5, 10, 20})
  {
   SCOPED_TRACE(offset);
   size_t count = 1;
   ASSERT_NO_THROW(count = file.pread(&data, 0, offset));
   EXPECT_EQ(count, 0);
   EXPECT_EQ(data, '?');
  }
  ASSERT_EQ(file.pread(&data, 1, 0), 1);
  EXPECT_EQ(data, 'a');
 }

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
