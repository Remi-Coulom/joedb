#include "joedb/journal/Encoded_File.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Brotli_Codec.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/interpreter/Database_Schema.h"

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

  EXPECT_EQ(file.get_size(), 0);

  const int32_t value = 0x01020304;
  file.write<int32_t>(value);
  file.write<int32_t>(value);
  file.set_position(0);

  EXPECT_EQ(file.get_size(), 8);

  EXPECT_EQ(file.read<int32_t>(), value);
  file.set_position(0);

  EXPECT_EQ(file.get_size(), 8);

  EXPECT_EQ(file.read<int8_t>(), 0x04);
  EXPECT_EQ(file.read<int8_t>(), 0x03);
  EXPECT_EQ(file.read<int8_t>(), 0x02);
  EXPECT_EQ(file.read<int8_t>(), 0x01);
  EXPECT_EQ(file.read<int32_t>(), value);
 }

#ifdef JOEDB_HAS_BROTLI
 ////////////////////////////////////////////////////////////////////////////
 TEST(Encoded_File, brotli)
 ////////////////////////////////////////////////////////////////////////////
 {
  Brotli_Codec codec;
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
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Encoded_File, brotli_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  Brotli_Codec codec;
  Memory_File db_file;
  encoded_file::Generic_File_Database db(db_file);
  Encoded_File file(codec, db);

  const size_t N = 10000;
  const std::string table_name(N, 'x');

  {
   Writable_Journal journal(file);
   journal.create_table(table_name);
   journal.default_checkpoint();
  }

  EXPECT_TRUE(size_t(db_file.get_size()) < N);
  EXPECT_TRUE(size_t(file.get_size()) > N);

  {
   Readonly_Journal journal(file);
   Database_Schema schema;
   journal.replay_log(schema);
   EXPECT_EQ(schema.get_tables().size(), 1);
   EXPECT_EQ(schema.get_tables().begin()->second, table_name);
  }
 }
#endif
}
