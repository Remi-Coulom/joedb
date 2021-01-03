#include "joedb/journal/Generic_File.h"

#include "gtest/gtest.h"

#include <sstream>
#include <cstdio>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void polymorphic_readonly_test(Generic_File &file)
 ////////////////////////////////////////////////////////////////////////////
 {
  EXPECT_EQ(file.get_size(), 4);
  EXPECT_EQ(file.read<int32_t>(), 1234);
  file.set_position(2);
  EXPECT_EQ(file.read<int16_t>(), 0);
  file.set_position(0);
  EXPECT_EQ(file.read<int16_t>(), 1234);
  file.commit();
 }

 ////////////////////////////////////////////////////////////////////////////
 void polymorphic_test(Generic_File &file)
 ////////////////////////////////////////////////////////////////////////////
 {
  file.write<int32_t>(1234);
  file.set_position(0);
  file.commit();
  polymorphic_readonly_test(file);
 }

 ////////////////////////////////////////////////////////////////////////////
 void large_write_test(Generic_File &file)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (sizeof(size_t) > 4)
  {
   const size_t n = (1ULL << 29) + 10000ULL;
   const uint64_t magic = 1234567890ULL;
   std::vector<uint64_t> buffer(n);
   std::fill_n(buffer.begin(), n, magic);

   file.write_data((char *)buffer.data(), n * sizeof(uint64_t));

   std::fill_n(buffer.begin(), n, 0);
   file.set_position(0);
   file.read_data((char *)buffer.data(), n * sizeof(uint64_t));

   for (size_t i = 0; i < n; i += 1000)
    EXPECT_EQ(buffer[i], magic);
  }
 }
}

#include "joedb/journal/Readonly_Memory_File.h"
/////////////////////////////////////////////////////////////////////////////
TEST(Polymorphic_File, Readonly_Memory_File)
/////////////////////////////////////////////////////////////////////////////
{
 const uint8_t memory[4] = {0xd2, 0x04, 0x00, 0x00};
 joedb::Readonly_Memory_File file(memory, 4);
 joedb::polymorphic_readonly_test(file);
}

#include "joedb/journal/Memory_File.h"
/////////////////////////////////////////////////////////////////////////////
TEST(Polymorphic_File, Memory_File)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 polymorphic_test(file);
}

#include "joedb/journal/Stream_File.h"
/////////////////////////////////////////////////////////////////////////////
TEST(Polymorphic_File, Stream_File)
/////////////////////////////////////////////////////////////////////////////
{
 std::stringstream stream;

 {
  joedb::Stream_File file(stream, joedb::Open_Mode::write_existing);
  polymorphic_test(file);
 }

 std::istringstream istream(stream.str());
 joedb::Input_Stream_File readonly_file(istream);
 joedb::polymorphic_readonly_test(readonly_file);
}

#include "joedb/journal/Portable_File.h"
/////////////////////////////////////////////////////////////////////////////
TEST(Polymorphic_File, Portable_File)
/////////////////////////////////////////////////////////////////////////////
{
 const char * file_name = "portable_file_test.joedb";
 {
  joedb::Portable_File file(file_name, joedb::Open_Mode::create_new);
  polymorphic_test(file);
 }
 std::remove(file_name);
}
#if 0
/////////////////////////////////////////////////////////////////////////////
TEST(Polymorphic_File, Portable_File_large_write)
/////////////////////////////////////////////////////////////////////////////
{
 const char * file_name = "portable_file_test.joedb";
 {
  joedb::Portable_File file(file_name, joedb::Open_Mode::create_new);
  large_write_test(file);
 }
 std::remove(file_name);
}
#endif

#include "joedb/journal/File.h"
/////////////////////////////////////////////////////////////////////////////
TEST(Polymorphic_File, File)
/////////////////////////////////////////////////////////////////////////////
{
 const char * file_name = "file_test.joedb";
 {
  joedb::File file(file_name, joedb::Open_Mode::create_new);
  polymorphic_test(file);
 }
 std::remove(file_name);
}
/////////////////////////////////////////////////////////////////////////////
TEST(Polymorphic_File, File_large_write)
/////////////////////////////////////////////////////////////////////////////
{
 const char * file_name = "file_test.joedb";
 {
  joedb::File file(file_name, joedb::Open_Mode::create_new);
  large_write_test(file);
 }
 std::remove(file_name);
}
