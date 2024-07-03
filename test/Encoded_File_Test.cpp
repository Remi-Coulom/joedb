#include "joedb/journal/Encoded_File.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Brotli_Codec.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/interpreter/Database_Schema.h"

#include "gtest/gtest.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 TEST(Encoded_File, overwrite)
 ////////////////////////////////////////////////////////////////////////////
 {
  Codec codec;
  Memory_File db_file;
  encoded_file::Generic_File_Database db(db_file);
  Encoded_File file(codec, db);

  file.seek(0);
  file.pos_write("xxx", 3);
  file.seek(0);
  file.pos_write("yyy", 3);
  file.seek(0);

  char data[6];
  const size_t size = file.pos_read(data, 6);
  EXPECT_EQ(size, 3);
  EXPECT_EQ(data[0], 'y');
  EXPECT_EQ(data[1], 'y');
  EXPECT_EQ(data[2], 'y');
 }

 ////////////////////////////////////////////////////////////////////////////
 static void encoded_file_test(Codec &codec)
 ////////////////////////////////////////////////////////////////////////////
 {
  const int32_t value = 0x01020304;

  Memory_File db_file;

  {
   encoded_file::Generic_File_Database db(db_file);
   Encoded_File file(codec, db);

   EXPECT_EQ(file.get_size(), 0);

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

  {
   encoded_file::Generic_File_Database db(db_file);
   Encoded_File file(codec, db);
   EXPECT_EQ(file.get_size(), 8);
   EXPECT_EQ(file.read<int8_t>(), 0x04);
   EXPECT_EQ(file.read<int8_t>(), 0x03);
   EXPECT_EQ(file.read<int8_t>(), 0x02);
   EXPECT_EQ(file.read<int8_t>(), 0x01);
   EXPECT_EQ(file.read<int32_t>(), value);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 static void encoded_journal_test(Codec &codec, bool is_compression)
 ////////////////////////////////////////////////////////////////////////////
 {
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

  if (is_compression)
  {
   EXPECT_TRUE(size_t(db_file.get_size()) < N);
  }

  EXPECT_TRUE(size_t(file.get_size()) > N);

  {
   Readonly_Journal journal(file);
   Database_Schema schema;
   journal.replay_log(schema);
   EXPECT_EQ(schema.get_tables().size(), 1);
   EXPECT_EQ(schema.get_tables().begin()->second, table_name);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Encoded_File, identity_codec)
 ////////////////////////////////////////////////////////////////////////////
 {
  Codec codec;
  encoded_file_test(codec);
  encoded_journal_test(codec, false);
 }


#ifdef JOEDB_HAS_BROTLI
 ////////////////////////////////////////////////////////////////////////////
 TEST(Encoded_File, brotli_codec)
 ////////////////////////////////////////////////////////////////////////////
 {
  Brotli_Codec codec;
  encoded_file_test(codec);
  encoded_journal_test(codec, true);
 }
#endif
}
