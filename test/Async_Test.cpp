#include "joedb/journal/Async_Reader.h"
#include "joedb/journal/Async_Writer.h"
#include "joedb/journal/File.h"

#include "gtest/gtest.h"

#include <cstdio>

namespace joedb
{
 static constexpr size_t blocks = 250;
 static constexpr size_t step = 7;
 static constexpr size_t block_size = 1 << 13;
 static constexpr size_t reads = 200 * blocks;
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

  reader.raw_pread(&buffer[0], 4, 8);
  EXPECT_EQ(buffer, "zzzz");
  reader.raw_pread(&buffer[0], 4, 4);
  EXPECT_EQ(buffer, "yyyy");
  reader.raw_pread(&buffer[0], 4, 0);
  EXPECT_EQ(buffer, "xxxx");

  writer.write("aaaabbbbcccc", 12);

  reader.raw_pread(&buffer[0], 4, 16);
  EXPECT_EQ(buffer, "bbbb");

  reader.read(&buffer[0], 4);
  EXPECT_EQ(buffer, "xxxx");

  std::remove(file_name);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Async, perf_with_pread)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::remove(file_name);

  std::vector<char> buffer(block_size);

  File file (file_name, Open_Mode::create_new);
  for (size_t i = 0; i < blocks; i++)
   file.write_data(buffer.data(), buffer.size());

  Async_Reader reader(file, 0, blocks * block_size);

  for (size_t i = 0; i < reads; i++)
  {
   const size_t offset = block_size * ((i * step) % blocks);
   reader.raw_pread(buffer.data(), buffer.size(), offset);
  }

  std::remove(file_name);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Async, perf_with_seek_and_read)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::remove(file_name);

  std::vector<char> buffer(block_size);

  File file (file_name, Open_Mode::create_new);
  for (size_t i = 0; i < blocks; i++)
   file.write_data(buffer.data(), buffer.size());

  Async_Reader reader(file, 0, blocks * block_size);

  for (size_t i = 0; i < reads; i++)
  {
   const size_t offset = block_size * ((i * step) % blocks);
   reader.seek_and_read(buffer.data(), buffer.size(), offset);
  }

  std::remove(file_name);
 }
}
