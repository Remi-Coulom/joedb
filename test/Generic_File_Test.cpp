#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Readonly_Memory_File.h"
#include "joedb/journal/Async_Reader.h"
#include "joedb/journal/Async_Writer.h"

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
TEST(Generic_File, slice)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 file.write<uint64_t>(1);
 file.write<uint64_t>(2);
 file.write<uint64_t>(3);

 file.set_position(0);
 EXPECT_EQ(file.read<uint64_t>(), 1ULL);
 EXPECT_EQ(file.get_size(), 24);

 file.set_slice(8, 8);
 file.set_position(0);
 EXPECT_EQ(file.read<uint64_t>(), 2ULL);
 EXPECT_EQ(file.get_size(), 8);
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
 file.set_position(-1);
 EXPECT_EQ(0, file.get_position());
}

/////////////////////////////////////////////////////////////////////////////
void check_data(int32_t *data, int32_t start, int32_t count)
/////////////////////////////////////////////////////////////////////////////
{
 for (int i = 0; i < count; i++)
 {
  if (joedb::is_big_endian())
   joedb::Generic_File::change_endianness(data[i]);
  EXPECT_EQ(start + i, data[i]);
 }
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

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, async)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 for (int32_t i = 0; i < 10000; i++)
  file.write<int32_t>(i);
 file.flush();

 joedb::Async_Reader reader1(file, 0, 128);
 joedb::Async_Reader reader2(file, 128, 256);
 joedb::Async_Writer writer(file, 40000);

 joedb::Memory_File file1;
 joedb::Memory_File file2;

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
  EXPECT_EQ(file.read<int32_t>(), i);
  EXPECT_EQ(file1.read<int32_t>(), i);
  EXPECT_EQ(file2.read<int32_t>(), i + 32);
 }

 file.set_position(40000);
 for (int32_t i = 4; --i >= 0;)
  EXPECT_EQ(file.read<int32_t>(), i);
}
