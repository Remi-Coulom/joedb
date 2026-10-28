#include "joedb/journal/Encoded_File.h"
#include "joedb/journal/Identity_Codec.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/interpreted/Database_Schema.h"

#ifdef JOEDB_HAS_BROTLI
#include "joedb/journal/Brotli_Codec.h"
#endif

#include "gtest/gtest.h"

#include <random>
#include <memory>

namespace joedb
{
 TEST(Encoded_File, invalid_buffer_sizes_do_not_hide_valid_data)
 {
  Identity_Codec codec;
  Memory_File storage;
  db::encoded_file::Writable_Database db(storage);
  const Blob blob = db.write_blob("ok");
  db.new_buffer(blob, -1, 0);
  db.new_buffer(blob, 0, 0);
  db.new_buffer(blob, (1 << 24) + 1, 0);
  db.new_buffer(blob, 2, 0);
  db.soft_checkpoint();
  Readonly_Encoded_File encoded(codec, db, storage);
  char data[2] = {};
  ASSERT_NO_THROW(encoded.full_pread(data, sizeof(data), 0));
  EXPECT_EQ(std::string_view(data, sizeof(data)), "ok");
 }

 TEST(Encoded_File, truncated_blob_is_reported)
 {
  Identity_Codec codec;
  Memory_File storage;
  db::encoded_file::Writable_Database db(storage);
  db.new_buffer(Blob(1000000, 2), 2, 0);
  db.soft_checkpoint();
  Readonly_Encoded_File encoded(codec, db, storage);
  char data[2];
  EXPECT_THROW(encoded.full_pread(data, sizeof(data), 0), Exception);
 }

 TEST(Encoded_File, out_of_order_writes)
 {
  Identity_Codec codec;
  Memory_File storage;
  db::encoded_file::Writable_Database db(storage);
  Encoded_File file(codec, db);
  file.pwrite("world", 5, 5);
  file.pwrite("hello", 5, 0);
  ASSERT_EQ(file.get_size(), 10);
  std::string result(10, '?');
  EXPECT_NO_THROW(file.full_pread(result.data(), result.size(), 0));
  EXPECT_EQ(result, "helloworld");
 }

 static void check_random_overwrites(Codec &codec)
 {
  Memory_File storage;
  std::string expected(8192, '\0');
  std::mt19937 random(0x5eed);
  {
   db::encoded_file::Writable_Database db(storage);
   Encoded_File file(codec, db);
   file.pwrite(expected.data(), expected.size(), 0);
   for (size_t step = 0; step < 160; ++step)
   {
    SCOPED_TRACE(step);
    const size_t offset = random() % (expected.size() + 1);
    const size_t count = 1 + random() % 257;
    std::string data(count, '\0');
    for (char &c: data)
     c = char(random() & 0xff);
    if (offset + count > expected.size())
     expected.resize(offset + count);
    expected.replace(offset, count, data);
    file.pwrite(data.data(), data.size(), int64_t(offset));
    EXPECT_EQ(file.get_size(), int64_t(expected.size()));
    if (step % 7 == 0)
    {
     std::string actual(expected.size(), '?');
     ASSERT_NO_THROW(file.full_pread(actual.data(), actual.size(), 0));
     ASSERT_EQ(actual, expected);
    }
   }
   file.sync();
  }
  {
   db::encoded_file::Writable_Database db(storage);
   Encoded_File file(codec, db);
   std::string actual(expected.size(), '?');
   ASSERT_NO_THROW(file.full_pread(actual.data(), actual.size(), 0));
   EXPECT_EQ(actual, expected);
  }
 }

 TEST(Encoded_File, identity_random_overwrites)
 {
  Identity_Codec codec;
  check_random_overwrites(codec);
 }

#ifdef JOEDB_HAS_BROTLI
 TEST(Encoded_File, brotli_random_overwrites)
 {
  Brotli_Codec codec;
  check_random_overwrites(codec);
 }
#endif

 TEST(Encoded_File, failed_encoding_can_be_retried)
 {
  class Failing_Codec: public Identity_Codec
  {
   public:
    bool fail = true;
    std::string encode(std::string_view data) override
    {
     if (fail)
      throw Exception("injected codec failure");
     return Identity_Codec::encode(data);
    }
  } codec;
  Memory_File storage;
  db::encoded_file::Writable_Database db(storage);
  Encoded_File file(codec, db);
  file.pwrite("retained", 8, 0);
  EXPECT_THROW(file.sync(), Exception);
  EXPECT_EQ(file.get_size(), 8);
  codec.fail = false;
  EXPECT_NO_THROW(file.sync());
  std::string data(8, '?');
  ASSERT_NO_THROW(file.full_pread(data.data(), data.size(), 0));
  EXPECT_EQ(data, "retained");
  EXPECT_EQ(db.get_buffer_table().get_size(), 1);
 }

 TEST(Encoded_File, destructor_contains_encoding_failure)
 {
  class Failed_Codec: public Identity_Codec
  {
   public:
    int attempts = 0;
    std::string encode(std::string_view) override
    {
     ++attempts;
     throw Exception("injected codec failure");
    }
  } codec;
  Memory_File storage;
  db::encoded_file::Writable_Database db(storage);
  auto file = std::make_unique<Encoded_File>(codec, db);
  file->pwrite("buffered", 8, 0);
  EXPECT_NO_THROW(file.reset());
  EXPECT_EQ(codec.attempts, 1);
  EXPECT_TRUE(db.get_buffer_table().is_empty());
 }

#ifdef JOEDB_HAS_BROTLI
 TEST(Encoded_File, invalid_compressed_input)
 {
  Brotli_Codec codec;
  const std::string encoded = codec.encode("valid payload");
  Decoder &decoder = codec;
  char output[32] = {};
  EXPECT_THROW(decoder.decode(encoded, output, 2), Exception);
  EXPECT_THROW(decoder.decode(encoded, output, sizeof(output)), Exception);
  EXPECT_THROW(decoder.decode("not brotli data", output, sizeof(output)), Exception);
 }
#endif

 ////////////////////////////////////////////////////////////////////////////
 TEST(Encoded_File, overwrite)
 ////////////////////////////////////////////////////////////////////////////
 {
  Identity_Codec codec;
  Memory_File db_file;
  db::encoded_file::Writable_Database db(db_file);
  Encoded_File file(codec, db);

  file.pwrite("xxx", 3, 0);
  file.pwrite("yyy", 3, 0);

  for (int i = 2; --i >= 0;)
  {
   char data[6];
   const size_t size = file.pread(data, 6, 0);
   EXPECT_EQ(size, 3);
   EXPECT_EQ(data[0], 'y');
   EXPECT_EQ(data[1], 'y');
   EXPECT_EQ(data[2], 'y');
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Encoded_File, large_write)
 ////////////////////////////////////////////////////////////////////////////
 {
  Identity_Codec codec;
  Memory_File db_file;
  db::encoded_file::Writable_Database db(db_file);
  Encoded_File file(codec, db);

  const size_t large_size = (1 << 24) + 123;
  std::string written(large_size, 0);
  for (size_t i = 0; i < written.size(); i++)
   written[i] = char(i & 0x7f);

  file.pwrite(written.data(), written.size(), 0);

  EXPECT_EQ(file.get_size(), int64_t(written.size()));

  std::string read(written.size(), 0);
  ASSERT_EQ(file.pread(read.data(), read.size(), 0), read.size());
  EXPECT_EQ(read, written);
 }

 ////////////////////////////////////////////////////////////////////////////
 static void encoded_file_test(Codec &codec)
 ////////////////////////////////////////////////////////////////////////////
 {
  const int32_t value = 0x01020304;

  Memory_File db_file;

  {
   db::encoded_file::Writable_Database db(db_file);
   Encoded_File file(codec, db);

   EXPECT_EQ(file.get_size(), 0);

   {
    File_Buffer file_buffer(file);
    file_buffer.write<int32_t>(value);
    file_buffer.write<int32_t>(value);
    file_buffer.flush();
   }

   EXPECT_EQ(file.get_size(), 8);

   {
    File_Buffer file_buffer(file);
    EXPECT_EQ(file_buffer.read<int32_t>(), value);
   }

   EXPECT_EQ(file.get_size(), 8);

   {
    File_Buffer file_buffer(file);
    EXPECT_EQ(file_buffer.read<int8_t>(), 0x04);
    EXPECT_EQ(file_buffer.read<int8_t>(), 0x03);
    EXPECT_EQ(file_buffer.read<int8_t>(), 0x02);
    EXPECT_EQ(file_buffer.read<int8_t>(), 0x01);
    EXPECT_EQ(file_buffer.read<int32_t>(), value);
   }
  }

  {
   db::encoded_file::Writable_Database db(db_file);
   Encoded_File file(codec, db);
   EXPECT_EQ(file.get_size(), 8);
   File_Buffer file_buffer(file);
   EXPECT_EQ(file_buffer.read<int8_t>(), 0x04);
   EXPECT_EQ(file_buffer.read<int8_t>(), 0x03);
   EXPECT_EQ(file_buffer.read<int8_t>(), 0x02);
   EXPECT_EQ(file_buffer.read<int8_t>(), 0x01);
   EXPECT_EQ(file_buffer.read<int32_t>(), value);
  }

  {
   db::encoded_file::Writable_Database db(db_file);
   Encoded_File file(codec, db);
   EXPECT_EQ(file.get_size(), 8);
   char buffer[8];
   EXPECT_EQ(file.pread(buffer, 8, 0), 8);
   EXPECT_EQ(file.pread(buffer, 8, 8), 0);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 static void encoded_journal_test(Codec &codec, bool is_compression)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File db_file;
  db::encoded_file::Writable_Database db(db_file);
  Encoded_File file(codec, db);

  const size_t N = 10000;
  const std::string table_name(N, 'x');

  {
   Writable_Journal journal(file);
   journal.create_table(table_name);
   journal.soft_checkpoint();
  }

  if (is_compression)
  {
   EXPECT_LT(size_t(db_file.get_size()),N);
  }

  EXPECT_GT(size_t(file.get_size()), N);

  {
   Readonly_Journal journal(file);
   Database_Schema schema;
   journal.replay_log(schema);
   EXPECT_EQ(schema.get_tables().size(), 1);
   EXPECT_EQ(schema.get_tables().begin()->second, table_name);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Encoded_File, Identity_Codec)
 ////////////////////////////////////////////////////////////////////////////
 {
  Identity_Codec codec;
  encoded_file_test(codec);
  encoded_journal_test(codec, false);
 }


#ifdef JOEDB_HAS_BROTLI
 ////////////////////////////////////////////////////////////////////////////
 TEST(Encoded_File, Brotli_Codec)
 ////////////////////////////////////////////////////////////////////////////
 {
  Brotli_Codec codec;
  encoded_file_test(codec);
  encoded_journal_test(codec, true);
 }
#endif
}
