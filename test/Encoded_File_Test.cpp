#include "joedb/journal/Encoded_File.h"
#include "joedb/journal/File.h"

#include "gtest/gtest.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 TEST(Encoded_File, basic)
 ////////////////////////////////////////////////////////////////////////////
 {
  Codec codec;

  Memory_File db_file;

  encoded_file::Generic_File_Database db(db_file);
  Encoded_File file(codec, db);

  const int32_t value = 0x01020304;
  file.write<int32_t>(value);
  file.set_position(0);
  EXPECT_EQ(file.read<int32_t>(), value);
  file.set_position(0);
  EXPECT_EQ(file.read<int8_t>(), 0x04);
  EXPECT_EQ(file.read<int8_t>(), 0x03);
  EXPECT_EQ(file.read<int8_t>(), 0x02);
  EXPECT_EQ(file.read<int8_t>(), 0x01);

  db.checkpoint();
 }
}
