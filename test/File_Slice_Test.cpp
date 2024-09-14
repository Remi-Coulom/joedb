#include "joedb/journal/File.h"

#ifdef JOEDB_FILE_IS_POSIX_FILE

#include "joedb/journal/File_Slice.h"

#include <gtest/gtest.h>

namespace joedb
{
 TEST(File_Slice, basic)
 {
  const char * const file_name = "test.joedb";
  std::remove(file_name);

  {
   File file(file_name, Open_Mode::create_new);
   file.write<int32_t>(1234);
   file.write<int32_t>(5678);
   file.write<int32_t>(9999);
   file.write<int32_t>(8765);
  }

  {
   const int fd = open(file_name, O_RDONLY);
   EXPECT_TRUE(fd >= 0);
   File_Slice file(fd, 8, 4);
   EXPECT_EQ(file.read<int32_t>(), 9999);
   EXPECT_ANY_THROW(file.read<int32_t>()); // end of file
   close(fd);
  }

  std::remove(file_name);
 }
}

#endif
