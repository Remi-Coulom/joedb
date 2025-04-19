#include "joedb/journal/File_View.h"
#include "joedb/journal/Memory_File.h"
#include "gtest/gtest.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 TEST(File_View, test)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;
  File_View file_view(file);

  file.write<int64_t>(1234);
  file.flush();
  EXPECT_EQ(file_view.read<int64_t>(), 1234);

  file.write<int64_t>(5678);
  file.flush();
  EXPECT_EQ(file_view.read<int64_t>(), 5678);

  EXPECT_EQ(file_view.get_size(), file.get_size());
 }
}
