#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Readonly_Memory_File.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Header.h"
#include "joedb/interpreted/Database.h"

#include "gtest/gtest.h"

#include <random>
#include <array>
#include "joedb/Multiplexer.h"

namespace joedb
{
 TEST(Journal, invalid_vector_start_reports_exception)
 {
  for (const index_t start: {-1, 5})
  {
   SCOPED_TRACE(start);
   Memory_File file;
   {
    Writable_Journal journal(file);
    journal.create_table("items");
    journal.add_field(Table_Id(1), "value", Type::int32());
    journal.insert_into(Table_Id(1), Record_Id(0));
    const int32_t value = 42;
    journal.update_vector_int32(Table_Id(1), Record_Id(start), Field_Id(1), 1, &value);
    journal.soft_checkpoint();
   }
   Readonly_Journal journal(file);
   Database db;
   try
   {
    journal.replay_log(db);
    FAIL() << "invalid vector start was accepted";
   }
   catch (const Exception &e)
   {
    EXPECT_STREQ(e.what(), "update_vector out of range");
   }
   if (start < 0)
   {
    Dummy_Writable writable;
    EXPECT_THROW(journal.replay_log(writable), Exception);
   }
  }
 }

 TEST(Journal, deterministic_sparse_round_trip)
 {
  struct Record {bool used = false; int32_t value = 0; std::string name;};
  std::array<Record, 64> model;
  std::mt19937 random(0x6a6f6564);
  Memory_File file;
  Writable_Journal journal(file);
  Database source;
  Multiplexer writable{source, journal};
  const Table_Id table{1};
  writable.create_table("items");
  writable.add_field(table, "value", Type::int32());
  writable.add_field(table, "name", Type::string());

  const auto check = [&]()
  {
   writable.soft_checkpoint();
   Readonly_Journal reader(file);
   Database restored;
   reader.replay_log(restored);
   for (size_t i = 0; i < model.size(); ++i)
   {
    const Record_Id id{index_t(i)};
    ASSERT_EQ(restored.is_used(table, id), model[i].used) << i;
    if (model[i].used)
    {
     ASSERT_EQ(restored.get_int32(table, id, Field_Id(1)), model[i].value) << i;
     ASSERT_EQ(restored.get_string(table, id, Field_Id(2)), model[i].name) << i;
    }
   }
  };

  for (size_t step = 0; step < 1200; ++step)
  {
   SCOPED_TRACE(step);
   const size_t first = random() % model.size();
   const bool insert = !model[first].used;
   size_t count = 1;
   while (count < 4 && first + count < model.size() &&
          model[first + count].used == model[first].used)
    ++count;
   const Record_Id id{index_t(first)};
   if (!insert && random() % 3 == 0)
   {
    writable.delete_vector(table, id, count);
    for (size_t i = 0; i < count; ++i)
     model[first + i].used = false;
   }
   else
   {
    if (insert)
     writable.insert_vector(table, id, count);
    std::vector<int32_t> values(count);
    std::vector<std::string> names(count);
    for (size_t i = 0; i < count; ++i)
    {
     values[i] = int32_t(random() & 0x7fffffff);
     names[i].resize(random() % 9);
     for (char &c: names[i])
      c = char(random() & 0xff);
     model[first + i] = {true, values[i], names[i]};
    }
    if (random() % 2 == 0)
    {
     writable.update_vector_int32(table, id, Field_Id(1), count, values.data());
     writable.update_vector_string(table, id, Field_Id(2), count, names.data());
    }
    else
    {
     for (size_t i = 0; i < count; ++i)
     {
      writable.update_int32(table, id + i, Field_Id(1), values[i]);
      writable.update_string(table, id + i, Field_Id(2), names[i]);
     }
    }
   }
   if (step % 31 == 0)
    check();
  }
  check();
 }

 class Lock_Failure_File: public Memory_File
 {
  public:
   bool head_locked = false;
   bool tail_locked = false;
   bool fail_head_lock = false;
   bool fail_read = false;
   bool fail_size = false;
   int tail_unlock_count = 0;

   explicit Lock_Failure_File(Open_Mode mode = Open_Mode::shared_write): Memory_File(mode) {}

   void shared_lock(int64_t, int64_t) override
   {
    head_locked = true;
   }

   void exclusive_lock(int64_t start, int64_t) override
   {
    if (start == 0)
    {
     if (fail_head_lock)
      throw Exception("head lock failed");
     head_locked = true;
    }
    else
     tail_locked = true;
   }

   void unlock(int64_t start, int64_t) noexcept override
   {
    if (start == 0)
     head_locked = false;
    else
    {
     tail_locked = false;
     ++tail_unlock_count;
    }
   }

   int64_t get_size() const override
   {
    if (fail_size)
     throw Exception("size read failed");
    return Memory_File::get_size();
   }

   size_t pread(char *data, size_t size, int64_t offset) const override
   {
    if (fail_read)
     throw Exception("header read failed");
    return Memory_File::pread(data, size, offset);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, lock_pull_failure_releases_tail)
 ////////////////////////////////////////////////////////////////////////////
 {
  for (bool use_lock_object: {false, true})
  {
   SCOPED_TRACE(use_lock_object);
   Lock_Failure_File file;
   Writable_Journal journal(file);
   file.tail_unlock_count = 0;
   file.fail_read = true;

   if (use_lock_object)
    EXPECT_THROW(Tail_Exclusive_Lock{journal}, Exception);
   else
    EXPECT_THROW(journal.lock_pull(), Exception);

   EXPECT_FALSE(file.tail_locked);
   EXPECT_EQ(file.tail_unlock_count, 1);

   file.fail_read = false;
   journal.lock_pull();
   EXPECT_TRUE(file.tail_locked);
   journal.unlock();
   EXPECT_FALSE(file.tail_locked);
   EXPECT_EQ(file.tail_unlock_count, 2);
  }
 }

 TEST(Journal, moving_tail_lock_transfers_unlock_responsibility)
 {
  Lock_Failure_File file;
  Writable_Journal journal(file);
  file.tail_unlock_count = 0;
  {
   Tail_Exclusive_Lock original(journal);
   EXPECT_TRUE(file.tail_locked);
   {
    Tail_Exclusive_Lock moved(std::move(original));
    EXPECT_TRUE(file.tail_locked);
    EXPECT_EQ(file.tail_unlock_count, 0);
   }
   EXPECT_FALSE(file.tail_locked);
   EXPECT_EQ(file.tail_unlock_count, 1);
  }
  EXPECT_EQ(file.tail_unlock_count, 1);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, construction_head_lock_failure_releases_tail)
 ////////////////////////////////////////////////////////////////////////////
 {
  Lock_Failure_File file;
  file.fail_head_lock = true;
  EXPECT_THROW(Journal_Construction_Lock{file}, Exception);
  EXPECT_FALSE(file.tail_locked);
  EXPECT_EQ(file.tail_unlock_count, 1);

  file.fail_head_lock = false;
  {
   Journal_Construction_Lock lock(file);
   EXPECT_TRUE(file.tail_locked);
  }
  EXPECT_FALSE(file.tail_locked);
  EXPECT_EQ(file.tail_unlock_count, 2);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, construction_size_failure_releases_locks)
 ////////////////////////////////////////////////////////////////////////////
 {
  for (Open_Mode mode: {Open_Mode::read_existing, Open_Mode::shared_write, Open_Mode::write_existing})
  {
   SCOPED_TRACE(int(mode));
   Lock_Failure_File file(mode);
   file.fail_size = true;
   EXPECT_THROW(Journal_Construction_Lock{file}, Exception);
   EXPECT_FALSE(file.head_locked);
   EXPECT_FALSE(file.tail_locked);
   EXPECT_EQ(file.tail_unlock_count, mode == Open_Mode::shared_write ? 1 : 0);

   file.fail_size = false;
   {
    Journal_Construction_Lock lock(file);
    EXPECT_TRUE(file.head_locked);
    EXPECT_EQ(file.tail_locked, mode == Open_Mode::shared_write);
   }
   EXPECT_FALSE(file.head_locked);
   EXPECT_FALSE(file.tail_locked);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, construction_reads_size_after_locking)
 ////////////////////////////////////////////////////////////////////////////
 {
  class Growing_File: public Lock_Failure_File
  {
   public:
    std::string committed_data;

    void exclusive_lock(int64_t start, int64_t size) override
    {
     Lock_Failure_File::exclusive_lock(start, size);
     if (start != 0 && !committed_data.empty())
     {
      get_data() = std::move(committed_data);
      committed_data.clear();
     }
    }
  } file;

  {
   Writable_Journal journal(file);
  }

  Memory_File writer_file;
  writer_file.get_data() = file.get_data();
  int64_t committed_checkpoint;
  {
   Writable_Journal writer(writer_file);
   writer.comment("committed before lock acquisition completed");
   writer.soft_checkpoint();
   committed_checkpoint = writer.get_checkpoint();
  }
  file.committed_data = writer_file.get_data();

  {
   Writable_Journal opener(file);
   EXPECT_EQ(opener.get_checkpoint(), committed_checkpoint);
  }
  Readonly_Journal reopened(file);
  EXPECT_EQ(reopened.get_checkpoint(), committed_checkpoint);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, checkpoint)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;
  std::array<int64_t, 4> checkpoints;

  {
   Writable_Journal journal(file);
   EXPECT_EQ(journal.get_checkpoint(), 41);
   EXPECT_EQ(journal.get_hard_checkpoint(), 41);

   file.pread((char *)&checkpoints, sizeof(checkpoints), 0);
   EXPECT_EQ(checkpoints[0], 41);
   EXPECT_EQ(checkpoints[1], 41);
   EXPECT_EQ(checkpoints[2], 41);
   EXPECT_EQ(checkpoints[3], 41);
  }

  {
   Writable_Journal journal(file);

   journal.create_table("person");
   journal.soft_checkpoint();

   file.pread((char *)&checkpoints, sizeof(checkpoints), 0);
   EXPECT_EQ(checkpoints[0], 41);
   EXPECT_EQ(checkpoints[1], -49);
   EXPECT_EQ(checkpoints[2], 41);
   EXPECT_EQ(checkpoints[3], 41);

   journal.soft_checkpoint();

   file.pread((char *)&checkpoints, sizeof(checkpoints), 0);
   EXPECT_EQ(checkpoints[0], 41);
   EXPECT_EQ(checkpoints[1], -49);
   EXPECT_EQ(checkpoints[2], 41);
   EXPECT_EQ(checkpoints[3], 41);

   EXPECT_EQ(journal.get_checkpoint(), 49);
   EXPECT_EQ(journal.get_hard_checkpoint(), 41);
   journal.comment("Hello");
   journal.soft_checkpoint();

   file.pread((char *)&checkpoints, sizeof(checkpoints), 0);
   EXPECT_EQ(checkpoints[0], -56);
   EXPECT_EQ(checkpoints[1], -49);
   EXPECT_EQ(checkpoints[2], 41);
   EXPECT_EQ(checkpoints[3], 41);

   EXPECT_EQ(journal.get_checkpoint(), 56);
   EXPECT_EQ(journal.get_hard_checkpoint(), 41);
   journal.hard_checkpoint();

   file.pread((char *)&checkpoints, sizeof(checkpoints), 0);
   EXPECT_EQ(checkpoints[0], 56);
   EXPECT_EQ(checkpoints[1], 56);
   EXPECT_EQ(checkpoints[2], 41);
   EXPECT_EQ(checkpoints[3], 41);

   journal.hard_checkpoint();

   file.pread((char *)&checkpoints, sizeof(checkpoints), 0);
   EXPECT_EQ(checkpoints[0], 56);
   EXPECT_EQ(checkpoints[1], 56);
   EXPECT_EQ(checkpoints[2], 41);
   EXPECT_EQ(checkpoints[3], 41);

   EXPECT_EQ(journal.get_checkpoint(), 56);
   EXPECT_EQ(journal.get_hard_checkpoint(), 56);
  }

  Writable_Journal journal(file);
  EXPECT_EQ(journal.get_checkpoint(), 56);
  EXPECT_EQ(journal.get_hard_checkpoint(), 56);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, seek)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;
  {
   File_Buffer file_buffer(file);
   file_buffer.write<uint64_t>(0);
   file_buffer.write<uint64_t>(0);
   file_buffer.write<uint64_t>(41);
   file_buffer.write<uint64_t>(41);
   file_buffer.write<uint32_t>(Readonly_Journal::format_version);
   file_buffer.write<char>('j');
   file_buffer.write<char>('o');
   file_buffer.write<char>('e');
   file_buffer.write<char>('d');
   file_buffer.write<char>('b');
   file_buffer.set_position(0);
  }

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

  Readonly_Journal journal
  (
   Journal_Construction_Lock(file, Recovery::ignore_header)
  );
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

  {
   File_Buffer file_buffer(file);
   file_buffer.set_position(41);
   file_buffer.write<uint64_t>(0);
   file_buffer.flush();
  }

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
 TEST(Journal, overwrite)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;

  {
   Writable_Journal journal(file);
   journal.comment("Hello");
   journal.soft_checkpoint();
   journal.create_table("persan");
   journal.flush();
  }

  EXPECT_ANY_THROW(Writable_Journal{file});

  {
   Writable_Journal journal
   (
    Journal_Construction_Lock(file, Recovery::overwrite)
   );
   Dummy_Writable writable;
   journal.play_until_checkpoint(writable);
   journal.create_table("person");
   journal.soft_checkpoint();
  }

  Writable_Journal journal(file);
  Database db;
  journal.replay_log(db);
  EXPECT_EQ(db.get_tables().size(), 1);
  EXPECT_EQ(db.get_table_name(Table_Id{1}), "person");
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, overwrite_checkpoint)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;

  int64_t checkpoint;

  {
   Writable_Journal journal(file);
   journal.comment("Hello");
   journal.soft_checkpoint();
   checkpoint = journal.get_checkpoint();
   journal.comment("Hi");
   journal.soft_checkpoint();
   file.get_data().resize(checkpoint);
   EXPECT_EQ(file.get_size(), checkpoint);
  }

  {
   Readonly_Journal journal(file);
   EXPECT_EQ(journal.get_checkpoint(), checkpoint);
   journal.pull();
   Database db;
   EXPECT_ANY_THROW(journal.replay_log(db));
  }

  {
   Writable_Journal journal(file);
   EXPECT_EQ(journal.get_checkpoint(), checkpoint);
  }

  {
   Readonly_Journal journal(file);
   EXPECT_EQ(journal.get_checkpoint(), checkpoint);
   journal.pull();
   Database db;
   journal.replay_log(db);
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

  {
   File_Buffer file_buffer(file);
   file_buffer.write<int64_t>(correct_checkpoint);
   file_buffer.write<int64_t>(correct_checkpoint);
   file_buffer.write<int64_t>(1234);
   file_buffer.write<int64_t>(5678);
   file_buffer.flush();
  }

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
   journal.comment("Hello");
   journal.soft_checkpoint();
  }

  {
   const char bug = -1;
   file.pwrite(&bug, 1, Header::size);
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

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, pull)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!File::lockable)
   GTEST_SKIP();

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
  }

  std::remove("test.joedb");
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, shared_pull_performance)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!File::lockable)
   GTEST_SKIP();

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
  if (!File::lockable)
   GTEST_SKIP();

  std::remove("test.joedb");

  {
   File file("test.joedb", Open_Mode::shared_write);
   Writable_Journal journal(file);
  }

  std::remove("test.joedb");
 }

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
  EXPECT_ANY_THROW(Readonly_Journal journal{file});
  Writable_Journal journal{file};
  EXPECT_EQ(journal.get_checkpoint(), Header::size);
  EXPECT_EQ(file.get_size(), Header::ssize);
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

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, range_errors)
 ////////////////////////////////////////////////////////////////////////////
 {
  {
   Memory_File file;
   {
    Writable_Journal journal(file);
    journal.create_table("person");
    journal.add_field(Table_Id{1}, "age", joedb::Type::int32());
    journal.insert_into(Table_Id{1}, Record_Id{0});
    journal.update_int32(Table_Id{1}, Record_Id{0}, Field_Id{1}, 42);
    journal.soft_checkpoint();
   }
   {
    Readonly_Journal journal(file);
    Dummy_Writable writable;
    journal.replay_log(writable);
   }
  }

  {
   Memory_File file;
   {
    Writable_Journal journal(file);
    journal.create_table("person");
    journal.add_field(Table_Id{1}, "age", joedb::Type::int32());
    journal.insert_vector(Table_Id{1}, Record_Id{0}, 1);
    const int32_t n = 42;
    journal.update_vector_int32(Table_Id{1}, Record_Id{0}, Field_Id{1}, 1, &n);
    journal.soft_checkpoint();
   }
   {
    Readonly_Journal journal(file);
    Dummy_Writable writable;
    journal.replay_log(writable);
   }
   {
    Readonly_Journal journal(file);
    Database db;
    journal.replay_log(db);
   }
  }

  {
   Memory_File file;
   {
    Writable_Journal journal(file);
    journal.create_table("person");
    journal.add_field(Table_Id{1}, "age", joedb::Type::int32());
    journal.insert_vector(Table_Id{1}, Record_Id{0}, 1);
    const int32_t n[2] = {42, 43};
    journal.update_vector_int32(Table_Id{1}, Record_Id{0}, Field_Id{1}, 2, n);
    journal.soft_checkpoint();
   }
   {
    Readonly_Journal journal(file);
    Dummy_Writable writable;
    journal.replay_log(writable);
   }
   {
    Readonly_Journal journal(file);
    Database db;
    EXPECT_ANY_THROW(journal.replay_log(db));
   }
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Journal, deadlock_in_Windows)
 ////////////////////////////////////////////////////////////////////////////
 {
  const char * const file_name = "deadlock.joedb.tmp";
  std::remove(file_name);

  {
   File file(file_name, Open_Mode::create_new);
   Writable_Journal journal(file);
  }

  {
   File file(file_name, Open_Mode::write_existing_or_create_new);
   Writable_Journal journal(Journal_Construction_Lock{file, Recovery::overwrite});
  }

  std::remove(file_name);
 }
}
