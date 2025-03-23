#include "joedb/journal/Test_File.h"
#include "joedb/journal/Readonly_Memory_File.h"
#include "joedb/journal/Async_Reader.h"
#include "joedb/journal/Async_Writer.h"
#include "joedb/journal/Stream_File.h"

#include "gtest/gtest.h"

#include <sstream>

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, copy)
/////////////////////////////////////////////////////////////////////////////
{
 const uint64_t magic = 1234567;

 joedb::Test_File file;
 file.write<uint64_t>(magic);

 joedb::Test_File copy;
 file.copy_to(copy, 0, sizeof(uint64_t));
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
 file.copy_to(copy, sizeof(uint64_t), sizeof(uint64_t) * count);
 copy.set_position(0);
 EXPECT_EQ(copy.read<uint64_t>(), ~magic);

 for (size_t i = 0; i < count; i++)
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
 EXPECT_EQ(4, file.get_position());
 EXPECT_ANY_THROW(file.read<uint32_t>());
 EXPECT_EQ(4, file.get_position());
 EXPECT_ANY_THROW(file.read<uint32_t>());
 EXPECT_EQ(4, file.get_position());

 file.set_position(0);
 EXPECT_EQ(0, file.get_position());
 EXPECT_EQ(0, file.read<uint8_t>());
 EXPECT_EQ(1, file.read<uint8_t>());
 EXPECT_EQ(2, file.read<uint8_t>());
 EXPECT_EQ(3, file.read<uint8_t>());
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
 EXPECT_ANY_THROW(file.read<int32_t>());

 file.set_position(4);
 file.read_data((char *)data.data(), sizeof(int32_t) * buffer_size / 2);
 check_data(data.data(), 1, buffer_size / 2);

 file.read_data((char *)data.data(), sizeof(int32_t) * buffer_size);
 check_data(data.data(), 1 + buffer_size / 2, buffer_size);

 EXPECT_ANY_THROW
 (
  file.read_data((char *)data.data(), sizeof(int32_t) * file_size)
 );
}

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, async)
/////////////////////////////////////////////////////////////////////////////
{
#if 1
 joedb::Test_File file;
#else
 joedb::File file("file.joedb", joedb::Open_Mode::create_new);
#endif

 for (int32_t i = 0; i < 10000; i++)
  file.write<int32_t>(i);
 file.flush();

 joedb::Async_Reader reader1(file, 0, 128);
 joedb::Async_Reader reader2(file, 128, 256);
 joedb::Async_Writer writer(file, 40000);

#if 1
 joedb::Test_File file1;
 joedb::Test_File file2;
#else
 joedb::File file1("file1.joedb", joedb::Open_Mode::create_new);
 joedb::File file2("file2.joedb", joedb::Open_Mode::create_new);
#endif

 joedb::Async_Writer writer1(file1, 0);
 joedb::Async_Writer writer2(file2, 0);

 const size_t buffer_size = 32;
 char buffer[buffer_size];

 for (int32_t i = 4; --i >= 0;)
 {
  {
   const size_t n1 = reader1.read(buffer, buffer_size);
   EXPECT_EQ(n1, buffer_size);
   writer1.write(buffer, n1);
  }

  {
   const size_t n2 = reader2.read(buffer, buffer_size);
   EXPECT_EQ(n2, buffer_size);
   writer2.write(buffer, n2);
  }

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

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, flush_and_position)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 file.write<int>(1234);
 file.write<int>(5678);
 file.flush();
 file.set_position(0);
 EXPECT_EQ(0, file.get_position());
 EXPECT_EQ(1234, file.read<int>());
 EXPECT_EQ(file.get_position(), 4);
 file.flush();
 EXPECT_EQ(file.get_position(), 8);
}

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, compact_read_bug)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 file.write<uint8_t>(0x12);
 file.write<uint8_t>(0xc0);
 file.write<uint8_t>(0x00);
 file.set_position(0);

 EXPECT_EQ(file.read<uint8_t>(), 0x12);
 EXPECT_ANY_THROW(file.compact_read<int8_t>());
}

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, position)
/////////////////////////////////////////////////////////////////////////////
{
 std::stringbuf sb;
 joedb::Stream_File file(sb, joedb::Open_Mode::write_existing);

 file.write<char>(0);
 file.write<char>(1);
 file.write<char>(2);
 file.write<char>(3);

 file.set_position(0);

 joedb::Async_Reader reader(file, 1, 3);
 char buffer[2];
 reader.read(buffer, 2);
 EXPECT_EQ(buffer[0], 1);
 EXPECT_EQ(buffer[1], 2);
 EXPECT_EQ(file.read<char>(), 0);
}

/////////////////////////////////////////////////////////////////////////////
TEST(Generic_File, get_position)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;

 EXPECT_EQ(file.get_position(), 0);
 file.write<char>(0);
 EXPECT_EQ(file.get_position(), 1);
 file.write<char>(1);
 EXPECT_EQ(file.get_position(), 2);

 file.set_position(0);
 EXPECT_EQ(file.get_position(), 0);
 file.read<char>();
 EXPECT_EQ(file.get_position(), 1);
 file.read<char>();
 EXPECT_EQ(file.get_position(), 2);
}
