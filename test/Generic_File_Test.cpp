#include "joedb/journal/Test_File.h"
#include "joedb/journal/Readonly_Memory_File.h"
#include "joedb/journal/Async_Reader.h"
#include "joedb/journal/Async_Writer.h"

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, copy)
/////////////////////////////////////////////////////////////////////////////
{
 const uint64_t magic = 1234567;

 joedb::Test_File file;
 file.write<uint64_t>(magic);

 joedb::Test_File copy;
 copy.copy(file, 0, std::numeric_limits<int64_t>::max());
 copy.set_position(0);

 EXPECT_EQ(copy.read<uint64_t>(), magic);
}

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, large_copy)
/////////////////////////////////////////////////////////////////////////////
{
 const uint64_t magic = 1234567;
 const size_t count = 100000;

 joedb::Memory_File file;
 for (size_t i = 0; i < 1 + 2 * count; i++)
  file.write<uint64_t>(magic);

 joedb::Memory_File copy;
 copy.write<uint64_t>(~magic);
 copy.copy(file, sizeof(uint64_t), sizeof(uint64_t) * count);
 copy.set_position(0);
 EXPECT_EQ(copy.read<uint64_t>(), ~magic);

 for (size_t i = 0; i < count; i++)
  EXPECT_EQ(copy.read<uint64_t>(), magic);
}

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, slice)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Test_File file;
 file.write<uint64_t>(1);
 file.write<uint64_t>(2);
 file.write<uint64_t>(3);

 file.set_position(0);
 EXPECT_EQ(file.read<uint64_t>(), 1ULL);
 EXPECT_EQ(file.get_size(), 24);

 file.set_slice(8, 8);
 file.set_position(0);
 EXPECT_EQ(file.get_position(), 0);
 EXPECT_EQ(file.read<uint64_t>(), 2ULL);
 EXPECT_EQ(file.get_size(), 8);
}

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, slice_pread_pwrite)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Test_File file;
 file.write<uint64_t>(1);
 file.write<uint64_t>(2);
 file.write<uint64_t>(3);

 file.set_slice(8, 16);
 EXPECT_EQ(file.get_position(), 0);
 EXPECT_EQ(file.read<uint64_t>(), 2ULL);
 file.set_position(0);
 EXPECT_EQ(file.read<uint64_t>(), 2ULL);

 {
  uint64_t x;
  file.pos_pread((char *)&x, sizeof(x), 0);
  EXPECT_EQ(x, 2ULL);
  file.pos_pread((char *)&x, sizeof(x), 8);
  EXPECT_EQ(x, 3ULL);
 }

 const uint64_t six = 6;
 file.pos_pwrite((const char *)&six, sizeof(six), 8);

 const uint64_t five = 5;
 file.pos_pwrite((const char *)&five, sizeof(five), 0);

 file.set_position(0);
 EXPECT_EQ(file.read<uint64_t>(), 5ULL);
 EXPECT_EQ(file.read<uint64_t>(), 6ULL);
}

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, readonly_memory_file)
/////////////////////////////////////////////////////////////////////////////
{
 const char data[] = {0, 1, 2, 3};
 const size_t size = sizeof(data);

 joedb::Readonly_Memory_File file(data, size);

 EXPECT_EQ(file.read<uint32_t>(), 0x03020100UL);
 file.read<uint32_t>();
 file.read<uint32_t>();

 file.set_position(0);
 EXPECT_EQ(0, file.get_position());
 EXPECT_ANY_THROW(file.set_position(-1));
 EXPECT_EQ(0, file.get_position());
}

/////////////////////////////////////////////////////////////////////////////
static void check_data(int32_t *data, int32_t start, int32_t count)
/////////////////////////////////////////////////////////////////////////////
{
 for (int i = 0; i < count; i++)
 {
  ASSERT_EQ(start + i, data[i]);
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, read_data)
/////////////////////////////////////////////////////////////////////////////
{
 const size_t buffer_size = (1 << 10);
 const int32_t file_size = int32_t(buffer_size) * 10;

 joedb::Test_File file;
 for (int32_t i = 0; i < file_size; i++)
  file.write<int32_t>(i);

 std::vector<int32_t> data(file_size);

 file.set_position(0);
 file.read_data((char *)data.data(), sizeof(int32_t) * file_size);
 check_data(data.data(), 0, file_size);
 EXPECT_FALSE(file.is_end_of_file());
 file.read<int32_t>();
 EXPECT_TRUE(file.is_end_of_file());

 file.set_position(4);
 file.read_data((char *)data.data(), sizeof(int32_t) * buffer_size / 2);
 check_data(data.data(), 1, buffer_size / 2);
 EXPECT_FALSE(file.is_end_of_file());

 file.read_data((char *)data.data(), sizeof(int32_t) * buffer_size);
 check_data(data.data(), 1 + buffer_size / 2, buffer_size);
 EXPECT_FALSE(file.is_end_of_file());

 file.read_data((char *)data.data(), sizeof(int32_t) * file_size);
 EXPECT_TRUE(file.is_end_of_file());
}

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, async)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Test_File file;
 for (int32_t i = 0; i < 10000; i++)
  file.write<int32_t>(i);
 file.flush();

 joedb::Async_Reader reader1(file, 0, 128);
 joedb::Async_Reader reader2(file, 128, 256);
 joedb::Async_Writer writer(file, 40000);

 joedb::Test_File file1;
 joedb::Test_File file2;

 joedb::Async_Writer writer1(file1, 0);
 joedb::Async_Writer writer2(file2, 0);

 const size_t buffer_size = 32;
 char buffer[buffer_size];

 for (int32_t i = 4; --i >= 0;)
 {
  writer1.write(buffer, reader1.read(buffer, buffer_size));
  writer2.write(buffer, reader2.read(buffer, buffer_size));
  writer.write((char *)&i, sizeof(int32_t));
 }

 file.set_position(0);
 file1.set_position(0);
 file2.set_position(0);

 for (int i = 0; i < 32; i++)
 {
  ASSERT_EQ(file.read<int32_t>(), i);
  ASSERT_EQ(file1.read<int32_t>(), i);
  ASSERT_EQ(file2.read<int32_t>(), i + 32);
 }

 file.set_position(40000);
 for (int32_t i = 4; --i >= 0;)
  EXPECT_EQ(file.read<int32_t>(), i);
}

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, async_more_capacity)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Test_File file;
 for (int32_t i = 0; i < 10000; i++)
  file.write<int32_t>(i);
 file.flush();

 joedb::Async_Reader reader(file, 4, 8);
 const size_t buffer_size = 16;
 char buffer[buffer_size];

 EXPECT_EQ(4UL, reader.read(buffer, buffer_size));
}
