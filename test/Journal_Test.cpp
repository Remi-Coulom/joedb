#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Readonly_Memory_File.h"
#include "joedb/journal/File.h"
#include "joedb/interpreted/Database.h"

#include "gtest/gtest.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, checkpoint)
 ////////////////////////////////////////////////////////////////////////////
 {
  joedb::Memory_File file;

  {
   joedb::Writable_Journal journal(file);
  }

  {
   joedb::Writable_Journal journal(file);

   journal.append();
   journal.create_table("person");
   journal.checkpoint(joedb::Commit_Level::no_commit);
  }

  joedb::Writable_Journal journal(file);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, seek)
 ////////////////////////////////////////////////////////////////////////////
 {
  joedb::Memory_File file;
  file.write<char>('j');
  file.write<char>('o');
  file.write<char>('e');
  file.write<char>('d');
  file.write<char>('b');
  file.write<uint32_t>(4);
  file.write<uint64_t>(0);
  file.write<uint64_t>(0);
  file.write<uint64_t>(41);
  file.write<uint64_t>(41);
  file.set_position(0);

  {
   joedb::Readonly_Journal journal(file);
   EXPECT_TRUE(journal.at_end_of_file());
   EXPECT_EQ(4UL, journal.get_file_version());
   EXPECT_EQ(41, journal.get_position());

   journal.set_position(0);
   EXPECT_FALSE(journal.at_end_of_file());
  }

  {
   joedb::Readonly_Memory_File readonly_file
   (
    file.get_data().data(),
    file.get_data().size()
   );
   joedb::Readonly_Journal journal2(readonly_file);
   EXPECT_ANY_THROW(joedb::Writable_Journal journal(readonly_file));
  }

  {
   joedb::Writable_Journal journal(file);
   EXPECT_EQ(4UL, journal.get_file_version());
   EXPECT_EQ(41, journal.get_position());
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, does_not_start_by_joedb)
 ////////////////////////////////////////////////////////////////////////////
 {
  joedb::Memory_File file;
  file.write<int>(1234);
  file.set_position(0);

  try
  {
   joedb::Readonly_Journal journal(file);
   FAIL() << "Should have thrown an exception";
  }
  catch (const Exception &e)
  {
   EXPECT_STREQ(e.what(), "File does not start by 'joedb'");
  }

  file.set_position(0);
  joedb::Readonly_Journal journal(file, joedb::Readonly_Journal::Check::none);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, unsupported_format_version)
 ////////////////////////////////////////////////////////////////////////////
 {
  joedb::Memory_File file;
  file.write<char>('j');
  file.write<char>('o');
  file.write<char>('e');
  file.write<char>('d');
  file.write<char>('b');
  file.write<uint32_t>(0);
  file.set_position(0);

  try
  {
   joedb::Readonly_Journal journal(file);
   FAIL() << "Should have thrown an exception";
  }
  catch (const Exception &e)
  {
   EXPECT_STREQ(e.what(), "Unsupported format version");
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, checkpoint_mismatch)
 ////////////////////////////////////////////////////////////////////////////
 {
  joedb::Memory_File file;
  file.write<char>('j');
  file.write<char>('o');
  file.write<char>('e');
  file.write<char>('d');
  file.write<char>('b');
  file.write<uint32_t>(4);
  file.write<uint64_t>(1);
  file.write<uint64_t>(2);
  file.write<uint64_t>(3);
  file.write<uint64_t>(4);
  file.set_position(0);

  try
  {
   joedb::Readonly_Journal journal(file);
   EXPECT_EQ(journal.get_checkpoint_position(), 41);
  }
  catch (const Exception &)
  {
   FAIL() << "Mismatched checkpoint is OK for read-only";
  }

  try
  {
   joedb::Writable_Journal journal(file);
   FAIL() << "Should have thrown an exception";
  }
  catch (const Exception &e)
  {
   EXPECT_STREQ(e.what(), "Checkpoint mismatch");
  }

  try
  {
   joedb::Writable_Journal journal(file, joedb::Readonly_Journal::Check::overwrite);
   EXPECT_EQ(journal.get_checkpoint_position(), 41);
  }
  catch (const Exception &)
  {
   FAIL() << "Mismatched checkpoint is OK with Check::overwrite";
  }

  try
  {
   joedb::Writable_Journal journal(file);
   EXPECT_EQ(journal.get_checkpoint_position(), 41);
  }
  catch (const Exception &)
  {
   FAIL() << "checkpoints should be matching now";
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, checkpoint_different_from_file_size)
 ////////////////////////////////////////////////////////////////////////////
 {
  joedb::Memory_File file;
  file.write<char>('j');
  file.write<char>('o');
  file.write<char>('e');
  file.write<char>('d');
  file.write<char>('b');
  file.write<uint32_t>(4);
  file.write<uint64_t>(0);
  file.write<uint64_t>(0);
  file.write<uint64_t>(42);
  file.write<uint64_t>(42);
  file.set_position(0);

  try
  {
   joedb::Readonly_Journal journal(file);
   FAIL() << "Should have thrown an exception";
  }
  catch (const Exception &e)
  {
   EXPECT_STREQ(e.what(), "Checkpoint is bigger than file size");
  }

  file.set_position(0);
  file.write<char>('j');
  file.write<char>('o');
  file.write<char>('e');
  file.write<char>('d');
  file.write<char>('b');
  file.write<uint32_t>(4);
  file.write<uint64_t>(0);
  file.write<uint64_t>(0);
  file.write<uint64_t>(41);
  file.write<uint64_t>(41);
  file.write<uint64_t>(0);
  file.set_position(0);

  {
   joedb::Readonly_Journal journal(file);
  }

  try
  {
   joedb::Writable_Journal journal(file);
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
  joedb::Memory_File file;

  int64_t correct_checkpoint;

  {
   joedb::Writable_Journal journal(file);
   journal.create_table("person");
   journal.default_checkpoint();
   correct_checkpoint = journal.get_checkpoint_position();
   journal.create_table("country");
   journal.default_checkpoint();
  }

  file.set_position(9);
  file.write<int64_t>(correct_checkpoint);
  file.write<int64_t>(correct_checkpoint);
  file.write<int64_t>(1234);
  file.write<int64_t>(5678);

  {
   joedb::Database db;
   joedb::Readonly_Journal journal(file);
   EXPECT_EQ(journal.get_checkpoint_position(), correct_checkpoint);
   journal.replay_log(db);
   EXPECT_EQ(db.get_tables().size(), 1);
  }

  EXPECT_ANY_THROW(joedb::Writable_Journal{file});

  {
   joedb::Writable_Journal journal{file, joedb::Readonly_Journal::Check::overwrite};
   EXPECT_EQ(journal.get_checkpoint_position(), correct_checkpoint);
   journal.append();
   journal.create_table("city");
   journal.default_checkpoint();
  }

  {
   joedb::Readonly_Journal journal(file);
   joedb::Database db;
   journal.replay_log(db);
   EXPECT_EQ(db.get_tables().size(), 2);
   EXPECT_EQ(db.get_tables().begin()->second, "person");
   EXPECT_EQ((++db.get_tables().begin())->second, "city");
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, unexpected_operation)
 ////////////////////////////////////////////////////////////////////////////
 {
  joedb::Memory_File file;
  file.write<char>('j');
  file.write<char>('o');
  file.write<char>('e');
  file.write<char>('d');
  file.write<char>('b');
  file.write<uint32_t>(4);
  file.write<uint64_t>(0);
  file.write<uint64_t>(0);
  file.write<uint64_t>(42);
  file.write<uint64_t>(42);
  file.write<uint8_t>(255);
  file.set_position(0);

  joedb::Readonly_Journal journal(file);
  joedb::Database db;

  try
  {
   journal.replay_log(db);
   FAIL() << "Should have thrown an exception";
  }
  catch (const Exception &e)
  {
   EXPECT_STREQ(e.what(), "Unexpected operation: file.get_position() = 42");
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, construction_with_create_new)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::remove("test.joedb");

  {
   joedb::File file("test.joedb", joedb::Open_Mode::create_new);
   joedb::Writable_Journal journal(file);
   EXPECT_EQ(41, journal.get_checkpoint_position());
  }

  std::remove("test.joedb");
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, construction_with_read_existing)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::remove("test.joedb");

  {
   joedb::File file("test.joedb", joedb::Open_Mode::create_new);
   joedb::Writable_Journal journal(file);
  }

  {
   joedb::File file("test.joedb", joedb::Open_Mode::read_existing);
   joedb::Readonly_Journal journal(file);
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
   joedb::File file_1("test.joedb", joedb::Open_Mode::shared_write);
   joedb::File file_2("test.joedb", joedb::Open_Mode::shared_write);

   joedb::Writable_Journal journal_1(file_1);
   joedb::Writable_Journal journal_2(file_2);

   journal_1.valid_data();
   journal_1.checkpoint(joedb::Commit_Level::no_commit);

   EXPECT_TRUE
   (
    journal_1.get_checkpoint_position() > journal_2.get_checkpoint_position()
   );

   journal_2.pull();

   EXPECT_TRUE
   (
    journal_1.get_checkpoint_position() == journal_2.get_checkpoint_position()
   );

   EXPECT_TRUE
   (
    journal_2.get_position() < journal_1.get_position()
   );

   EXPECT_TRUE
   (
    journal_2.get_checkpoint_position() == journal_1.get_position()
   );

   {
    joedb::Writable_Journal::Tail_Writer tail_writer(journal_2);
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
   joedb::File file("test.joedb", joedb::Open_Mode::shared_write);
   joedb::Writable_Journal journal(file);

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
   joedb::File file("test.joedb", joedb::Open_Mode::shared_write);
   joedb::Writable_Journal journal(file);
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
   joedb::File file("test.joedb", joedb::Open_Mode::create_new);
   joedb::Writable_Journal journal(file);

   for (int i = 10000; --i >= 0;)
   {
    journal.valid_data();
    journal.checkpoint(joedb::Commit_Level::no_commit);
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
   joedb::File file("test.joedb", joedb::Open_Mode::create_new);
   joedb::Writable_Journal journal(file);

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
  joedb::Memory_File file;
 #else
  joedb::File file("test.joedb", joedb::Open_Mode::create_new);
 #endif

  {
   joedb::Writable_Journal journal(file);
   journal.comment("properly checkpointed comment");
   journal.default_checkpoint();
  }

  {
   joedb::Writable_Journal journal(file);
   journal.set_position(journal.get_checkpoint_position());
   journal.comment("uncheckpointed comment");
  }

  try
  {
   joedb::Writable_Journal journal(file);
   ADD_FAILURE();
  }
  catch(...)
  {
  }

  {
   joedb::Writable_Journal journal
   (
    file,
    joedb::Readonly_Journal::Check::overwrite
   );
   journal.set_position(journal.get_checkpoint_position());
   journal.comment("Overwriting the uncheckpointed comment");
   journal.default_checkpoint();
  }
 }

 ///////////////////////////////////////////////////////////////////////////
 TEST(Journal, reset_position_after_checkpoint)
 ///////////////////////////////////////////////////////////////////////////
 {
  std::remove("test.joedb");

  {
   joedb::File file("test.joedb", joedb::Open_Mode::create_new);
   joedb::Writable_Journal journal(file);
   EXPECT_EQ(journal.get_checkpoint_position(), 41);
   journal.comment(std::string(5000, 'A'));
   journal.default_checkpoint();
   EXPECT_EQ(journal.get_checkpoint_position(), 5044);
   journal.comment(std::string(5000, 'B'));
   journal.default_checkpoint();
   EXPECT_EQ(journal.get_checkpoint_position(), 10047);
  }

  {
   joedb::File file("test.joedb", joedb::Open_Mode::read_existing);
   joedb::Readonly_Journal journal(file);

   joedb::Memory_File copy_file;
   {
    joedb::Writable_Journal copy_journal(copy_file);

    journal.one_step(copy_journal);
    journal.pull();
    journal.one_step(copy_journal);

    copy_journal.default_checkpoint();
    EXPECT_EQ(copy_journal.get_checkpoint_position(), 10047);
   }

   joedb::Memory_File another_file;
   {
    joedb::Writable_Journal another_journal(another_file);
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
  joedb::Memory_File file;
  joedb::Writable_Journal journal(file);

  const int64_t initial = journal.get_checkpoint_position();
  journal.create_table("person");
  journal.default_checkpoint();
  const int64_t after_person = journal.get_checkpoint_position();
  journal.create_table("city");
  journal.default_checkpoint();
  const int64_t after_city = journal.get_checkpoint_position();

  EXPECT_TRUE(after_person > initial);
  EXPECT_TRUE(after_city > after_person);

  {
   joedb::Memory_File to_file;
   joedb::Writable_Journal to_journal(to_file);
   to_journal.pull_from(journal, initial);
   EXPECT_EQ(to_journal.get_checkpoint_position(), initial);
   to_journal.pull_from(journal, after_person);
   EXPECT_EQ(to_journal.get_checkpoint_position(), after_person);
   to_journal.pull_from(journal, after_city);
   EXPECT_EQ(to_journal.get_checkpoint_position(), after_city);
  }

  {
   joedb::Memory_File to_file;
   joedb::Writable_Journal to_journal(to_file);
   to_journal.pull_from(journal);
   EXPECT_EQ(to_journal.get_checkpoint_position(), after_city);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, empty_initial_file)
 ////////////////////////////////////////////////////////////////////////////
 {
  joedb::Memory_File file;
  joedb::Readonly_Journal journal{file};

  EXPECT_EQ
  (
   journal.get_checkpoint_position(),
   joedb::Readonly_Journal::header_size
  );

  EXPECT_EQ(file.get_size(), 0);

  file.write<char>('j');
  file.flush();
  EXPECT_ANY_THROW(joedb::Readonly_Journal{file});
 }
}
