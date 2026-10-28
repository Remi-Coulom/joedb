#include "joedb/journal/Stream_File.h"
#include "joedb/journal/File_Buffer.h"

#include "gtest/gtest.h"

#include <sstream>
#include <algorithm>

using namespace joedb;

TEST(Stream_File_Test, short_writes_and_sync_failure)
{
 class Short_Streambuf: public std::stringbuf
 {
  public:
   bool fail_sync = false;
   std::streamsize xsputn(const char *data, std::streamsize count) override
   {
    return std::stringbuf::xsputn(data, std::min<std::streamsize>(count, 2));
   }
   int sync() override {return fail_sync ? -1 : 0;}
 } buffer;
 Stream_File file(buffer, Open_Mode::create_new);
 file.pwrite("abcdefg", 7, 0);
 EXPECT_EQ(buffer.str(), "abcdefg");
 buffer.fail_sync = true;
 EXPECT_THROW(file.pwrite("z", 1, 0), Exception);
}

TEST(Stream_File_Test, failed_write)
{
 class Failed_Streambuf: public std::stringbuf
 {
   std::streamsize xsputn(const char *, std::streamsize) override {return 0;}
 } buffer;
 Stream_File file(buffer, Open_Mode::create_new);
 EXPECT_THROW(file.pwrite("abc", 3, 0), Exception);
}

/////////////////////////////////////////////////////////////////////////////
TEST(Stream_File_Test, append_after_get_size)
/////////////////////////////////////////////////////////////////////////////
{
 std::stringbuf stringbuf("ABC", std::ios::in | std::ios::out);
 Stream_File file(stringbuf, Open_Mode::write_existing);

 const int64_t end = file.get_size();
 ASSERT_EQ(end, 3);
 file.pwrite("D", 1, end);

 EXPECT_EQ("ABCD", stringbuf.str());
}

/////////////////////////////////////////////////////////////////////////////
TEST(Stream_File_Test, position_test)
{
 std::stringbuf stringbuf;

 Stream_File file(stringbuf, Open_Mode::create_new);
 File_Buffer file_buffer(file);
 EXPECT_EQ(0LL, file_buffer.get_position());

 const int64_t N = 100;
 for (int i = N; --i >= 0;)
  file_buffer.write<uint8_t>('x');
 EXPECT_EQ(N, file_buffer.get_position());

 file_buffer.set_position(-1);
 EXPECT_ANY_THROW(file_buffer.read<char>());
 file_buffer.set_position(0);

 const int64_t pos = 12;
 file_buffer.set_position(pos);
 EXPECT_EQ(pos, file_buffer.get_position());

 const uint8_t x = file_buffer.read<uint8_t>();
 EXPECT_EQ('x', x);
 EXPECT_EQ(pos + 1, file_buffer.get_position());

 file_buffer.set_position(0);
 file_buffer.write_data("xxxxxx", 7);
 file_buffer.set_position(0);
 EXPECT_EQ(file_buffer.read<char>(), 'x');
 EXPECT_EQ(file_buffer.read<char>(), 'x');
 EXPECT_EQ(file_buffer.read<char>(), 'x');
 EXPECT_EQ(file_buffer.read<char>(), 'x');
 EXPECT_EQ(file_buffer.read<char>(), 'x');
 EXPECT_EQ(file_buffer.read<char>(), 'x');
 EXPECT_EQ(file_buffer.read<char>(), 0);
}
