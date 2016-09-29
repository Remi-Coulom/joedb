#include "Multiplexer.h"
#include "File.h"
#include "Journal_File.h"
#include "Database.h"
#include "DB_Listener.h"

#include "gtest/gtest.h"

#include <sstream>

TEST(Multiplexer_Test, basic)
{
 {
  joedb::File file("multiplexer.joedb", joedb::File::mode_t::create_new);
  EXPECT_EQ(file.get_status(), joedb::File::status_t::success);
  joedb::Database db1;
  joedb::Database db2;

  joedb::Journal_File journal(file);
  joedb::DB_Listener db1_listener(db1);
  joedb::DB_Listener db2_listener(db2);

  joedb::Multiplexer multiplexer;
  joedb::Listener &journal_multiplexer = multiplexer.add_listener(journal);
  joedb::Listener &db1_multiplexer = multiplexer.add_listener(db1_listener);
  joedb::Listener &db2_multiplexer = multiplexer.add_listener(db2_listener);

  db1.set_listener(db1_multiplexer);
  db2.set_listener(db2_multiplexer);

  journal.replay_log(journal_multiplexer);

  EXPECT_EQ(0, db1.get_tables().size());
  EXPECT_EQ(0, db2.get_tables().size());

  db1.create_table("T");

  EXPECT_EQ(1, db1.get_tables().size());
  EXPECT_EQ(1, db2.get_tables().size());

  db2.create_table("U");

  EXPECT_EQ(2, db1.get_tables().size());
  EXPECT_EQ(2, db2.get_tables().size());
 }

 {
  joedb::File file("multiplexer.joedb", joedb::File::mode_t::read_existing);
  EXPECT_EQ(file.get_status(), joedb::File::status_t::success);

  joedb::Database db1;
  joedb::Database db2;

  joedb::Journal_File journal(file);
  joedb::DB_Listener db1_listener(db1);
  joedb::DB_Listener db2_listener(db2);

  joedb::Multiplexer multiplexer;
  joedb::Listener &journal_multiplexer = multiplexer.add_listener(journal);
  joedb::Listener &db1_multiplexer = multiplexer.add_listener(db1_listener);
  joedb::Listener &db2_multiplexer = multiplexer.add_listener(db2_listener);

  db1.set_listener(db1_multiplexer);
  db2.set_listener(db2_multiplexer);

  journal.replay_log(journal_multiplexer);

  EXPECT_EQ(2, db1.get_tables().size());
  EXPECT_EQ(2, db2.get_tables().size());
 }
}
