#include "joedb/journal/File.h"
#include "joedb/journal/Upgradable_File.h"
#include "joedb/journal/Buffered_File.h"

#include "gtest/gtest.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 TEST(Upgradable_File, basic)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::remove("test.joedb");

  EXPECT_ANY_THROW
  (
   Upgradable_File<File>("test.joedb", Open_Mode::read_existing)
  );

  {
   File file("test.joedb", Open_Mode::create_new);
   Buffered_File file_buffer(file);
   file_buffer.write<uint32_t>(1234);
   file_buffer.flush();
  }

  Upgradable_File<File> file("test.joedb", Open_Mode::read_existing);
  Buffered_File file_buffer(file);
  EXPECT_EQ(file_buffer.read<uint32_t>(), 1234);

  file_buffer.set_position(file_buffer.get_position());
  file_buffer.write<uint32_t>(5678);

  file_buffer.set_position(0);
  EXPECT_EQ(file_buffer.read<uint32_t>(), 1234);
  EXPECT_ANY_THROW(file_buffer.read<uint32_t>());

  std::remove("test.joedb");
 }
}
