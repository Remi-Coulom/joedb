#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Readonly_Memory_File.h"
#include "joedb/interpreter/Database.h"
#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(Journal, checkpoint)
/////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////
TEST(Journal, seek)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file(joedb::Open_Mode::read_existing);
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
 }

 {
  joedb::Writable_Journal journal(file);
  EXPECT_EQ(4UL, journal.get_file_version());
  EXPECT_EQ(41, journal.get_position());
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Journal, does_not_start_by_joedb)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file(joedb::Open_Mode::read_existing);
 file.write<int>(1234);
 file.set_position(0);

 try
 {
  joedb::Readonly_Journal journal(file);
  FAIL() << "Should have thrown an exception";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "File does not start by 'joedb'");
 }

 file.set_position(0);
 joedb::Readonly_Journal journal(file, true);
}

/////////////////////////////////////////////////////////////////////////////
TEST(Journal, unsupported_format_version)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file(joedb::Open_Mode::read_existing);
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
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "Unsupported format version");
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Journal, checkpoint_mismatch)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file(joedb::Open_Mode::read_existing);
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
  FAIL() << "Should have thrown an exception";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "Checkpoint mismatch");
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Journal, checkpoint_different_from_file_size)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file(joedb::Open_Mode::read_existing);
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
 catch (const joedb::Exception &e)
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

 try
 {
  joedb::Readonly_Journal journal(file);
  FAIL() << "Should have thrown an exception";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "Checkpoint is smaller than file size. This file may contain an aborted transaction. joedb_convert can be used to fix it.");
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Journal, unexpected_operation)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file(joedb::Open_Mode::read_existing);
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
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "Unexpected operation: file.get_position() = 42");
 }
}
