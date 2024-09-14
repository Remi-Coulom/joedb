#include "joedb/journal/File.h"
#include "joedb/journal/Upgradable_File.h"

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
   file.write<uint32_t>(1234);
   file.flush();
  }

  Upgradable_File<File> file("test.joedb", Open_Mode::read_existing);
  EXPECT_EQ(file.read<uint32_t>(), 1234);

  file.set_position(file.get_position());
  file.write<uint32_t>(5678);

  file.set_position(0);
  EXPECT_EQ(file.read<uint32_t>(), 1234);
  EXPECT_ANY_THROW(file.read<uint32_t>());

  std::remove("test.joedb");
 }
}
