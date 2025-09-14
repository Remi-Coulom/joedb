#include "joedb/journal/Stream_File.h"
#include "joedb/journal/File_Buffer.h"

#include "gtest/gtest.h"

#include <sstream>

using namespace joedb;

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
