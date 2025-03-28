#include "joedb/ui/File_Parser.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/interpreted/Database.h"

#include "gtest/gtest.h"

#include <fstream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 TEST(File_Parser, help)
 ////////////////////////////////////////////////////////////////////////////
 {
  File_Parser parser;
  std::ofstream out;
  parser.print_help(out);
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(File_Parser, memory)
 ////////////////////////////////////////////////////////////////////////////
 {
  File_Parser parser;
  std::ofstream out;

  int arg_index = 0;
  constexpr int argc = 1;
  const char * argv[argc] = {"memory"};
  parser.parse(out, argc, const_cast<char **>(argv), arg_index);
  Writable_Journal journal(*parser.get_file());
  journal.comment("Hello");
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(File_Parser, interpreted)
 ////////////////////////////////////////////////////////////////////////////
 {
  File_Parser parser;
  std::ofstream out;

  {
   int arg_index = 0;
   constexpr int argc = 2;
   const char * argv[argc] = {"interpreted", "dump_test.joedbi"};
   parser.parse(out, argc, const_cast<char **>(argv), arg_index);
  }

  {
   int arg_index = 0;
   constexpr int argc = 3;
   const char * argv[argc] = {"interpreted", "--read", "dump_test.joedbi"};
   parser.parse(out, argc, const_cast<char **>(argv), arg_index);
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
   std::ofstream out;

   int arg_index = 0;
   constexpr int argc = 2;
   const char * argv[argc] = {"brotli", file_name};

   parser.parse(out, argc, const_cast<char **>(argv), arg_index);
   Writable_Journal journal(*parser.get_file());
   journal.create_table(table_name);
   journal.default_checkpoint();
  }

  {
   File_Parser parser;
   std::ofstream out;

   int arg_index = 0;
   constexpr int argc = 3;
   const char * argv[argc] = {"brotli", "--read", file_name};

   parser.parse(out, argc, const_cast<char **>(argv), arg_index);
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
  std::ofstream out;

  int arg_index = 0;
  constexpr int argc = 2;
  const char * argv[argc] =
  {
   "curl",
   "https://github.com/Remi-Coulom/joedb/raw/master/test/endianness.joedb"
  };

  parser.parse(out, argc, const_cast<char **>(argv), arg_index);
  Readonly_Journal journal(*parser.get_file());
  Database db;
  journal.replay_log(db);

  ASSERT_EQ(db.get_tables().size(), 1);
  EXPECT_EQ(db.get_tables().begin()->second, "endian");
 }
#endif
}
