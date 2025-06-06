#include "joedb/journal/Stream_File.h"

#include "gtest/gtest.h"

#include <sstream>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(Stream_File_Test, position_test)
{
 std::stringbuf stringbuf;

 Stream_File file(stringbuf, Open_Mode::create_new);
 EXPECT_EQ(0LL, file.get_position());

 const int64_t N = 100;
 for (int i = N; --i >= 0;)
  file.write<uint8_t>('x');
 EXPECT_EQ(N, file.get_position());

 file.set_position(-1);
 EXPECT_ANY_THROW(file.read<char>());
 file.set_position(0);

 const int64_t pos = 12;
 file.set_position(pos);
 EXPECT_EQ(pos, file.get_position());

 const uint8_t x = file.read<uint8_t>();
 EXPECT_EQ('x', x);
 EXPECT_EQ(pos + 1, file.get_position());

 file.set_position(0);
 file.write_data("xxxxxx", 7);
 file.set_position(0);
 EXPECT_EQ(file.read<char>(), 'x');
 EXPECT_EQ(file.read<char>(), 'x');
 EXPECT_EQ(file.read<char>(), 'x');
 EXPECT_EQ(file.read<char>(), 'x');
 EXPECT_EQ(file.read<char>(), 'x');
 EXPECT_EQ(file.read<char>(), 'x');
 EXPECT_EQ(file.read<char>(), 0);
}
