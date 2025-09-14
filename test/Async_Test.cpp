#include "joedb/journal/Async_Reader.h"
#include "joedb/journal/Async_Writer.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/File_Iterator.h"

#include "gtest/gtest.h"

#include <cstdio>

namespace joedb
{
 static constexpr int64_t blocks = 250;
 static constexpr int64_t step = 7;
 static constexpr int64_t block_size = 1 << 12; // 1 << 13 triggers a false warning
 static constexpr int64_t reads = 200 * blocks;
 static constexpr const char *file_name = "server.joedb";

 ////////////////////////////////////////////////////////////////////////////
 TEST(Async, pread_and_pwrite_correctness)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::remove(file_name);

  File file(file_name, Open_Mode::create_new);

  Async_Writer writer(file, 0);

  writer.write("xxxxyyyyzzzz", 12);

  Async_Reader reader(file, 0, 12);

  std::string buffer(4, ' ');

  Abstract_File &af(file);

  file.pread(&buffer[0], 4, 8);
  EXPECT_EQ(buffer, "zzzz");
  file.pread(&buffer[0], 4, 4);
  EXPECT_EQ(buffer, "yyyy");
  file.pread(&buffer[0], 4, 0);
  EXPECT_EQ(buffer, "xxxx");

  writer.write("aaaabbbbcccc", 12);

  af.pread(&buffer[0], 4, 16);
  EXPECT_EQ(buffer, "bbbb");

  reader.read(&buffer[0], 4);
  EXPECT_EQ(buffer, "xxxx");

  std::remove(file_name);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Async, perf)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::remove(file_name);

  std::vector<char> buffer(block_size);

  File file(file_name, Open_Mode::create_new);
  File_Iterator file_iterator(file);
  for (size_t i = 0; i < blocks; i++)
   file_iterator.write(buffer.data(), buffer.size());

  Async_Reader reader(file, 0, blocks * block_size);

  for (int64_t i = 0; i < reads; i++)
  {
   const int64_t offset = block_size * ((i * step) % blocks);
   file.pread(buffer.data(), buffer.size(), offset);
  }

  std::remove(file_name);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Async, negative_pull)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;
  Async_Reader reader(file, 10, 0);
  EXPECT_EQ(reader.get_remaining(), 0);
  constexpr int64_t capacity = 16;
  char buffer[capacity];
  EXPECT_EQ(reader.read(buffer, capacity), 0UL);
 }
}
