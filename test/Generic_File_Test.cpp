#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Readonly_Memory_File.h"

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

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, readonly_memory_file)
/////////////////////////////////////////////////////////////////////////////
{
 const char data[] = {0, 1, 2, 3};
 const size_t size = sizeof(data);

 joedb::Readonly_Memory_File file(data, size);

 EXPECT_EQ(file.read<uint32_t>(), 0x03020100UL);
}
