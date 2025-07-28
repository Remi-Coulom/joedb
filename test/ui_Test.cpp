#include "joedb/ui/Client_Command_Processor.h"
#include "joedb/ui/Client_Parser.h"
#include "joedb/journal/File.h"

#include "gtest/gtest.h"

#include <fstream>

namespace joedb
{
 TEST(ui, client)
 {
  const std::array<const char *, 5> args
  {
   "--check",
   "none",
   "--db",
   "interpreted",
   "memory"
  };

  Arguments arguments(args.size(), args.data());

  Client_Parser parser
  (
   Open_Mode::shared_write,
   Client_Parser::DB_Type::none,
   arguments
  );

  {
   std::ostringstream out;
   parser.print_help(out);
  }

  auto *writable_client = dynamic_cast<Writable_Client *>(parser.get());
  ASSERT_TRUE(writable_client);
  Writable_Client_Command_Processor processor(*writable_client);

  std::ostringstream out;
  std::ifstream in("client_test.joedbi");

  processor.set_prompt(true);
  processor.main_loop(in, out);

  std::ifstream reference_file("client_test.joedbi.out");
  ASSERT_TRUE(reference_file);
  std::ostringstream reference_string;
  reference_string << reference_file.rdbuf();

  std::ofstream tmp("client_test.joedbi.tmp");
  tmp << out.str();

  EXPECT_EQ(reference_string.str(), out.str());
 }

 TEST(ui, file_client)
 {
  std::remove("test.joedb");

  const std::array<const char *, 8> args
  {
   "--check",
   "none",
   "--db",
   "interpreted",
   "test.joedb",
   "file",
   "interpreted",
   "connection_file.joedbi"
  };

  Arguments arguments(args.size(), args.data());

  const Open_Mode default_mode = File::lockable
   ? Open_Mode::shared_write
   : Open_Mode::write_existing_or_create_new;

  Client_Parser parser
  (
   default_mode,
   Client_Parser::DB_Type::none,
   arguments
  );

  ASSERT_TRUE(parser.has_file());

  auto *writable_client = dynamic_cast<Writable_Client *>(parser.get());
  ASSERT_TRUE(writable_client);
  Writable_Client_Command_Processor processor(*writable_client);

  std::ostringstream out;
  std::ifstream in("file_client_test.joedbi");

  processor.set_prompt(true);
  processor.main_loop(in, out);

  std::ifstream file_connection(args[7]);
  ASSERT_TRUE(file_connection);
  std::ostringstream file_connection_string;
  file_connection_string << file_connection.rdbuf();

  ASSERT_EQ
  (
   file_connection_string.str(),
   "create_table person\n"
   "add_field person name string\n"
   "insert_into person 0\n"
   "update person 0 name \"toto\"\n"
   "insert_into person 1\n"
   "update person 1 name \"titi\"\n"
   "\n"
  );

  std::remove(args[4]);
  std::remove(args[7]);
 }
}
