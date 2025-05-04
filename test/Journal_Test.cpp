#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Readonly_Memory_File.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Header.h"
#include "joedb/interpreted/Database.h"

#include "gtest/gtest.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, checkpoint)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;

  {
   Writable_Journal journal(file);
  }

  {
   Writable_Journal journal(file);

   journal.append();
   journal.create_table("person");
   journal.soft_checkpoint();
  }

  Writable_Journal journal(file);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, seek)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;
  file.write<uint64_t>(0);
  file.write<uint64_t>(0);
  file.write<uint64_t>(41);
  file.write<uint64_t>(41);
  file.write<uint32_t>(Readonly_Journal::format_version);
  file.write<char>('j');
  file.write<char>('o');
  file.write<char>('e');
  file.write<char>('d');
  file.write<char>('b');
  file.set_position(0);

  {
   Readonly_Memory_File readonly_file
   (
    file.get_data().data(),
    file.get_data().size()
   );
   Readonly_Journal journal2(readonly_file);
   EXPECT_ANY_THROW(Writable_Journal journal(readonly_file));
  }

  {
   Writable_Journal journal(file);
   EXPECT_EQ(41, journal.get_position());
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, missing_joedb_signature)
 ////////////////////////////////////////////////////////////////////////////
 {
  Header header;
  header.checkpoint.fill(Header::size);
  header.version = Readonly_Journal::format_version;
  header.signature = Header::joedb;
  header.signature[4] = 'B';

  Memory_File file;
  file.pwrite((const char *)&header, Header::size, 0);

  try
  {
   Readonly_Journal journal(file);
   FAIL() << "Should have thrown an exception";
  }
  catch (const Exception &e)
  {
   EXPECT_STREQ(e.what(), "missing joedb signature");
  }

  Readonly_Journal journal(Journal_Construction_Lock(file, true));
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, unsupported_format_version)
 ////////////////////////////////////////////////////////////////////////////
 {
  Header header;
  header.checkpoint.fill(Header::size);
  header.version = 0;
  header.signature = Header::joedb;

  Memory_File file;
  file.pwrite((const char *)&header, sizeof(header), 0);

  try
  {
   Readonly_Journal journal(file);
   FAIL() << "Should have thrown an exception";
  }
  catch (const Exception &e)
  {
   EXPECT_STREQ(e.what(), "unsupported file format version");
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, checkpoint_mismatch)
 ////////////////////////////////////////////////////////////////////////////
 {
  Header header;
  header.checkpoint = {1, 2, 3, 4};
  header.version = Readonly_Journal::format_version;
  header.signature = Header::joedb;

  Memory_File file;
  file.pwrite((const char *)&header, Header::size, 0);

  try
  {
   Readonly_Journal journal(file);
   EXPECT_EQ(journal.get_checkpoint(), 41);
  }
  catch (const Exception &e)
  {
   FAIL() << e.what();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, checkpoint_different_from_file_size)
 ////////////////////////////////////////////////////////////////////////////
 {
  Header header;
  header.checkpoint = {0, 0, 42, 42};
  header.version = Readonly_Journal::format_version;
  header.signature = Header::joedb;

  Memory_File file;
  file.pwrite((const char *)&header, Header::size, 0);

  try
  {
   Readonly_Journal journal(file);
   FAIL() << "Should have thrown an exception";
  }
  catch (const Exception &e)
  {
   EXPECT_STREQ(e.what(), "Checkpoint is bigger than file size");
  }

  header.checkpoint = {0, 0, 41, 41};
  file.pwrite((const char *)&header, Header::size, 0);
  file.set_position(41);
  file.write<uint64_t>(0);

  {
   Readonly_Journal journal(file);
  }

  try
  {
   Writable_Journal journal(file);
   FAIL() << "Should have thrown an exception";
  }
  catch (const Exception &e)
  {
   EXPECT_STREQ(e.what(), "Checkpoint (41) is smaller than file size (49). This file may contain an aborted transaction. 'joedb_push file.joedb file fixed.joedb' can be used to truncate it.");
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, crash_simulation)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;

  int64_t correct_checkpoint;

  {
   Writable_Journal journal(file);
   EXPECT_EQ(journal.get_position(), Header::size);
   journal.create_table("person");
   journal.soft_checkpoint();
   correct_checkpoint = journal.get_checkpoint();
   EXPECT_GT(correct_checkpoint, Header::ssize);
   journal.create_table("country");
   journal.soft_checkpoint();
  }

  file.set_position(0);
  file.write<int64_t>(correct_checkpoint);
  file.write<int64_t>(correct_checkpoint);
  file.write<int64_t>(1234);
  file.write<int64_t>(5678);
  file.flush();

  {
   Database db;
   Readonly_Journal journal(file);
   EXPECT_EQ(journal.get_position(), Header::size);
   EXPECT_EQ(journal.get_checkpoint(), correct_checkpoint);
   journal.replay_log(db);
   EXPECT_EQ(db.get_tables().size(), 1);
  }

  EXPECT_ANY_THROW(Writable_Journal{file});
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, unexpected_operation)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;

  {
   Writable_Journal journal(file);
   file.write<uint8_t>(255);
   journal.soft_checkpoint();
  }

  Readonly_Journal journal(file);
  Database db;

  try
  {
   journal.replay_log(db);
   FAIL() << "Should have thrown an exception";
  }
  catch (const Exception &e)
  {
   EXPECT_STREQ(e.what(), "Unexpected operation: get_position() = 42");
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, construction_with_create_new)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::remove("test.joedb");

  {
   File file("test.joedb", Open_Mode::create_new);
   Writable_Journal journal(file);
   EXPECT_EQ(41, journal.get_checkpoint());
  }

  std::remove("test.joedb");
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, construction_with_read_existing)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::remove("test.joedb");

  {
   File file("test.joedb", Open_Mode::create_new);
   Writable_Journal journal(file);
  }

  {
   File file("test.joedb", Open_Mode::read_existing);
   Readonly_Journal journal(file);
  }

  std::remove("test.joedb");
 }

 #ifdef JOEDB_FILE_IS_LOCKABLE
 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, pull)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::remove("test.joedb");

  {
   File file_1("test.joedb", Open_Mode::shared_write);
   File file_2("test.joedb", Open_Mode::shared_write);

   Writable_Journal journal_1(file_1);
   Writable_Journal journal_2(file_2);

   journal_1.valid_data();
   journal_1.soft_checkpoint();

   EXPECT_TRUE
   (
    journal_1.get_checkpoint() > journal_2.get_checkpoint()
   );

   journal_2.pull();

   EXPECT_TRUE
   (
    journal_1.get_checkpoint() == journal_2.get_checkpoint()
   );

   EXPECT_TRUE
   (
    journal_2.get_position() < journal_1.get_position()
   );

   EXPECT_TRUE
   (
    journal_2.get_checkpoint() == journal_1.get_position()
   );

   {
    Writable_Journal::Tail_Writer tail_writer(journal_2);
    tail_writer.finish();
   }

   EXPECT_TRUE(journal_2.get_position() < journal_1.get_position());
  }

  std::remove("test.joedb");
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, shared_pull_performance)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::remove("test.joedb");

  {
   File file("test.joedb", Open_Mode::shared_write);
   Writable_Journal journal(file);

   for (int i = 10000; --i >= 0;)
    journal.pull();
  }

  std::remove("test.joedb");
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, construction_with_shared_write)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::remove("test.joedb");

  {
   File file("test.joedb", Open_Mode::shared_write);
   Writable_Journal journal(file);
  }

  std::remove("test.joedb");
 }

 #endif

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, checkpoint_performance)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::remove("test.joedb");

  {
   File file("test.joedb", Open_Mode::create_new);
   Writable_Journal journal(file);

   for (int i = 10000; --i >= 0;)
   {
    journal.valid_data();
    journal.soft_checkpoint();
   }
  }

  std::remove("test.joedb");
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, pull_performance)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::remove("test.joedb");

  {
   File file("test.joedb", Open_Mode::create_new);
   Writable_Journal journal(file);

   for (int i = 10000; --i >= 0;)
    journal.pull();
  }

  std::remove("test.joedb");
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, check)
 ////////////////////////////////////////////////////////////////////////////
 {
 #if 1
  Memory_File file;
 #else
  File file("test.joedb", Open_Mode::create_new);
 #endif

  {
   Writable_Journal journal(file);
   journal.comment("properly checkpointed comment");
   journal.soft_checkpoint();
  }

  {
   Writable_Journal journal(file);
   journal.skip_directly_to(journal.get_checkpoint());
   journal.comment("uncheckpointed comment");
   journal.flush();
  }

  try
  {
   Writable_Journal journal(file);
   ADD_FAILURE();
  }
  catch(...)
  {
  }
 }

 ///////////////////////////////////////////////////////////////////////////
 TEST(Journal, reset_position_after_checkpoint)
 ///////////////////////////////////////////////////////////////////////////
 {
  std::remove("test.joedb");

  {
   File file("test.joedb", Open_Mode::create_new);
   Writable_Journal journal(file);
   EXPECT_EQ(journal.get_checkpoint(), 41);
   journal.comment(std::string(5000, 'A'));
   journal.soft_checkpoint();
   EXPECT_EQ(journal.get_checkpoint(), 5044);
   journal.comment(std::string(5000, 'B'));
   journal.soft_checkpoint();
   EXPECT_EQ(journal.get_checkpoint(), 10047);
  }

  {
   File file("test.joedb", Open_Mode::read_existing);
   Readonly_Journal journal(file);

   Memory_File copy_file;
   {
    Writable_Journal copy_journal(copy_file);

    journal.one_step(copy_journal);
    journal.pull();
    journal.one_step(copy_journal);

    copy_journal.soft_checkpoint();
    EXPECT_EQ(copy_journal.get_checkpoint(), 10047);
   }

   Memory_File another_file;
   {
    Writable_Journal another_journal(another_file);
    journal.replay_log(another_journal);
   }

   EXPECT_EQ(copy_file.get_data(), another_file.get_data());
  }

  std::remove("test.joedb");
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, pull_from)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;
  Writable_Journal journal(file);

  const int64_t initial = journal.get_checkpoint();
  journal.create_table("person");
  journal.soft_checkpoint();
  const int64_t after_person = journal.get_checkpoint();
  journal.create_table("city");
  journal.soft_checkpoint();
  const int64_t after_city = journal.get_checkpoint();

  EXPECT_TRUE(after_person > initial);
  EXPECT_TRUE(after_city > after_person);

  {
   Memory_File to_file;
   Writable_Journal to_journal(to_file);
   to_journal.pull_from(journal, initial);
   EXPECT_EQ(to_journal.get_checkpoint(), initial);
   to_journal.pull_from(journal, after_person);
   EXPECT_EQ(to_journal.get_checkpoint(), after_person);
   to_journal.pull_from(journal, after_city);
   EXPECT_EQ(to_journal.get_checkpoint(), after_city);
  }

  {
   Memory_File to_file;
   Writable_Journal to_journal(to_file);
   to_journal.pull_from(journal);
   EXPECT_EQ(to_journal.get_checkpoint(), after_city);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, empty_initial_file)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;
  Readonly_Journal journal{file};

  EXPECT_EQ
  (
   journal.get_checkpoint(),
   Header::size
  );

  EXPECT_EQ(file.get_size(), 0);

  file.write<char>('j');
  file.flush();
  EXPECT_ANY_THROW(Readonly_Journal{file});
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, write_blob)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;
  Writable_Journal journal(file);

  {
   const Blob blob = journal.write_blob("Hello");
   journal.flush();
   EXPECT_EQ(journal.get_file().read_blob(blob), "Hello");
  }

  {
   const Blob blob = journal.write_blob("");
   journal.flush();
   EXPECT_EQ(journal.get_file().read_blob(blob), "");
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, write_big_blob)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;
  Writable_Journal journal(file);

  const std::string big_string(10000, 'x');

  {
   const Blob blob = journal.write_blob(big_string);
   journal.flush();
   EXPECT_EQ(journal.get_file().read_blob(blob), big_string);
  }
 }
}
