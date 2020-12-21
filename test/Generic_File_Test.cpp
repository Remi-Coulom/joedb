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

/////////////////////////////////////////////////////////////////////////////
void check_data(const int32_t *data, int32_t start, int32_t count)
/////////////////////////////////////////////////////////////////////////////
{
 for (int i = 0; i < count; i++)
  EXPECT_EQ(start + i, data[i]);
}

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, read_data)
/////////////////////////////////////////////////////////////////////////////
{
 const size_t buffer_size = (1 << 10);
 const int32_t file_size = int32_t(buffer_size) * 10;

 joedb::Memory_File file;
 for (int32_t i = 0; i < file_size; i++)
  file.write<int32_t>(i);

 std::vector<int32_t> data(file_size);

 file.set_position(0);
 file.read_data((char *)&data[0], sizeof(int32_t) * file_size);
 check_data(&data[0], 0, file_size);
 EXPECT_FALSE(file.is_end_of_file());
 file.read<int32_t>();
 EXPECT_TRUE(file.is_end_of_file());

 file.set_position(4);
 file.read_data((char *)&data[0], sizeof(int32_t) * buffer_size / 2);
 check_data(&data[0], 1, buffer_size / 2);
 EXPECT_FALSE(file.is_end_of_file());

 file.read_data((char *)&data[0], sizeof(int32_t) * buffer_size);
 check_data(&data[0], 1 + buffer_size / 2, buffer_size);
 EXPECT_FALSE(file.is_end_of_file());

 file.read_data((char *)&data[0], sizeof(int32_t) * file_size);
 EXPECT_TRUE(file.is_end_of_file());
}
