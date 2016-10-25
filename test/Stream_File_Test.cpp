#include "Stream_File.h"
#include "gtest/gtest.h"

#include <random>
#include <sstream>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(Stream_File_Test, position_test)
{
 std::stringstream stream;

 Stream_File file(stream, Generic_File::mode_t::create_new);
 EXPECT_EQ(0ULL, file.get_position());

 file.set_position(uint64_t(-1));
 EXPECT_EQ(0ULL, file.get_position());

 const uint64_t N = 100;
 for (int i = N; --i >= 0;)
  file.write<uint8_t>('x');
 EXPECT_EQ(N, file.get_position());

 const uint64_t pos = 12;
 file.set_position(pos);
 EXPECT_EQ(pos, file.get_position());

 const uint8_t x = file.read<uint8_t>();
 EXPECT_EQ('x', x);
 EXPECT_EQ(pos + 1, file.get_position());
}
