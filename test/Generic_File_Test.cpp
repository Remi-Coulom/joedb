#include "joedb/journal/Memory_File.h"

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, copy)
/////////////////////////////////////////////////////////////////////////////
{
 const uint64_t magic = 1234567;

 joedb::Memory_File file;
 file.write<uint64_t>(magic);

 joedb::Memory_File copy;
 copy.copy(file);
 copy.set_position(0);

 EXPECT_EQ(copy.read<uint64_t>(), magic);
}
