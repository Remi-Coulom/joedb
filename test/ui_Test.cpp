#include "joedb/ui/Client_Command_Processor.h"
#include "joedb/ui/Parsed_Client.h"
#include "joedb/journal/File.h"
#include "joedb/ui/joedbi.h"
#include "joedb/ui/joedb_client.h"
#include "joedb/ui/joedb_convert.h"
#include "joedb/ui/joedb_logdump.h"
#include "joedb/ui/joedb_pack.h"
#include "joedb/ui/joedb_push.h"
#include "joedb/ui/joedb_to_json.h"
#include "joedb/ui/write_lock.h"

#if defined(JOEDB_HAS_ASIO) && __cplusplus >= 202002L
#include "joedb/ui/joedb_server.h"
#include "joedb/ui/joedb_multi_server.h"
#endif

#include "gtest/gtest.h"

#include <fstream>
#include <sstream>

namespace joedb
{
 TEST(ui, tool_help_streams)
 {
  int (*const tools[])
  (Arguments &, std::istream &, std::ostream &, std::ostream &) =
  {
   joedbi, client, convert, logdump, pack, push, joedb_to_json, lock,
#if defined(JOEDB_HAS_ASIO) && __cplusplus >= 202002L
   server, multi_server,
#endif
  };

  for (const auto tool: tools)
  {
   const char *args[] = {"tool"};
   Arguments arguments(1, args);
   std::istringstream in;
   std::ostringstream out;
   std::ostringstream err;

   EXPECT_EQ(tool(arguments, in, out, err), 1);
   EXPECT_TRUE(out.str().empty());
   EXPECT_EQ(err.str().find("usage: tool"), 0);
  }
 }

 TEST(ui, joedbi_streams)
 {
  const char *args[] = {"joedbi", "--log", "none", "memory"};
  Arguments arguments(4, args);
  std::istringstream in("echo on\ncreate_table person\n");
  std::ostringstream out;
  std::ostringstream err;

  EXPECT_EQ(joedbi(arguments, in, out, err), 0);
  EXPECT_NE(out.str().find("OK: create_table person\n"), std::string::npos);
  EXPECT_TRUE(err.str().empty());
 }

 TEST(ui, client_dump_stream)
 {
  const char *file_name = "client_dump_stream.joedb.tmp";
  std::remove(file_name);
  {
   File file(file_name, Open_Mode::create_new);
   Writable_Journal journal(file);
   journal.create_table("person");
   journal.soft_checkpoint();
  }

  const char *args[] =
  {
   "joedb_client", "--log", "none", "--db", "dump", "--read", file_name
  };
  Arguments arguments(7, args);
  std::istringstream in("quit\n");
  std::ostringstream out;
  std::ostringstream err;

  EXPECT_EQ(client(arguments, in, out, err), 0);
  EXPECT_NE(out.str().find("create_table person\n"), std::string::npos);
  EXPECT_TRUE(err.str().empty());
  std::remove(file_name);
 }

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

  Logger logger;

  std::ostringstream out;

  Parsed_Client parsed_client
  (
   logger,
   Open_Mode::shared_write,
   Parsed_Client::DB_Type::none,
   arguments,
   out
  );

  {
   std::ostringstream help;
   parsed_client.print_help(help);
  }

  auto *writable_client = dynamic_cast<Writable_Client *>(parsed_client.get());
  ASSERT_TRUE(writable_client);
  Writable_Client_Command_Processor processor(*writable_client);

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

  {
   Arguments arguments(args.size(), args.data());

   Logger logger;

   const Open_Mode default_mode = File::lockable
    ? Open_Mode::shared_write
    : Open_Mode::write_existing_or_create_new;

   std::ostringstream out;

   Parsed_Client parsed_client
   (
    logger,
    default_mode,
    Parsed_Client::DB_Type::none,
    arguments,
    out
   );

   ASSERT_TRUE(parsed_client.has_file());

   auto *writable_client = dynamic_cast<Writable_Client *>(parsed_client.get());
   ASSERT_TRUE(writable_client);
   Writable_Client_Command_Processor processor(*writable_client);

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
  }

  std::remove(args[4]);
  std::remove(args[7]);
 }
}
