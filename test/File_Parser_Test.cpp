#include "joedb/ui/File_Parser.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/iostream.h"
#include "joedb/interpreted/Database.h"

#include "gtest/gtest.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 TEST(File_Parser, help)
 ////////////////////////////////////////////////////////////////////////////
 {
  File_Parser parser;
  joedb::null_iostream out;
  parser.print_help(out);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(File_Parser, memory)
 ////////////////////////////////////////////////////////////////////////////
 {
  File_Parser parser;
  Logger logger;

  std::array argv{"test", "memory"};
  Arguments arguments(argv.size(), argv.data());
  ASSERT_TRUE(parser.parse(logger, arguments));
  Writable_Journal journal(*parser.get_file());
  journal.comment("Hello");
  journal.soft_checkpoint();
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(File_Parser, interpreted)
 ////////////////////////////////////////////////////////////////////////////
 {
  File_Parser parser;
  Logger logger;

  {
   std::array argv{"test", "interpreted", "dump_test.joedbi"};
   Arguments arguments(argv.size(), argv.data());
   parser.parse(logger, arguments);
  }

  {
   std::array argv{"test", "interpreted", "--read", "dump_test.joedbi"};
   Arguments arguments(argv.size(), argv.data());
   parser.parse(logger, arguments);
  }
 }

#ifdef JOEDB_HAS_BROTLI
 ////////////////////////////////////////////////////////////////////////////
 TEST(File_Parser, brotli)
 ////////////////////////////////////////////////////////////////////////////
 {
  const char *file_name = "brotli.joedb";
  const char *table_name = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaargh";
  std::remove(file_name);

  {
   File_Parser parser;
   Logger logger;

   std::array argv{"test", "brotli", file_name};
   Arguments arguments(argv.size(), argv.data());
   ASSERT_TRUE(parser.parse(logger, arguments));
   Writable_Journal journal(*parser.get_file());
   journal.create_table(table_name);
   journal.soft_checkpoint();
  }

  {
   File_Parser parser;
   Logger logger;

   std::array argv{"test", "brotli", "--read", file_name};
   Arguments arguments(argv.size(), argv.data());
   ASSERT_TRUE(parser.parse(logger, arguments));
   EXPECT_ANY_THROW(Writable_Journal{*parser.get_file()});
   Readonly_Journal journal{*parser.get_file()};
   Database db;
   journal.replay_log(db);
   ASSERT_EQ(db.get_tables().size(), 1);
   EXPECT_EQ(db.get_tables().begin()->second, table_name);
  }

  std::remove(file_name);
 }
#endif

#ifdef JOEDB_HAS_CURL
 ////////////////////////////////////////////////////////////////////////////
 TEST(File_Parser, curl)
 ////////////////////////////////////////////////////////////////////////////
 {
  File_Parser parser;
  Logger logger;

  std::array argv
  {
   "test",
   "curl",
   "https://www.joedb.org/test/v10/endianness.joedb"
  };
  Arguments arguments(argv.size(), argv.data());
  ASSERT_TRUE(parser.parse(logger, arguments));
  Readonly_Journal journal(*parser.get_file());
  Database db;
  journal.replay_log(db);

  ASSERT_EQ(db.get_tables().size(), 1);
  EXPECT_EQ(db.get_tables().begin()->second, "endian");
 }
#endif
}
