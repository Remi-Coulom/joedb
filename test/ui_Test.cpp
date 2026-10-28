#include "joedb/ui/Client_Command_Processor.h"
#include "joedb/ui/Parsed_Client.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/ui/joedbi.h"
#include "joedb/ui/joedb_client.h"
#include "joedb/ui/joedb_convert.h"
#include "joedb/ui/joedb_logdump.h"
#include "joedb/ui/joedb_pack.h"
#include "joedb/ui/joedb_push.h"
#include "joedb/ui/joedb_to_json.h"
#include "joedb/ui/write_lock.h"
#include "joedb/ui/inplace_pack.h"
#include "joedb/concurrency/File_Connection.h"

#if defined(JOEDB_HAS_ASIO) && __cplusplus >= 202002L
#include "joedb/ui/joedb_server.h"
#include "joedb/ui/joedb_multi_server.h"
#endif

#include "gtest/gtest.h"

#include <fstream>
#include <sstream>
#include <limits>
#include <initializer_list>
#include <filesystem>

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

 class Tool_Test: public testing::Test
 {
  protected:
   using Tool = int (*)(Arguments &, std::istream &, std::ostream &, std::ostream &);
   struct Result
   {
    int code;
    std::string out;
    std::string err;
   };

   static constexpr const char *input_file = "tool_input.joedb.tmp";
   static constexpr const char *output_file = "tool_output.joedb.tmp";
   int64_t schema_checkpoint = 0;
   int64_t first_checkpoint = 0;

   static Result run
   (
    Tool tool,
    std::initializer_list<const char *> options,
    const std::string &input = ""
   )
   {
    std::vector<const char *> argv{"tool"};
    argv.insert(argv.end(), options.begin(), options.end());
    Arguments arguments(int(argv.size()), argv.data());
    std::istringstream in(input);
    std::ostringstream out;
    std::ostringstream err;
    const int code = tool(arguments, in, out, err);
    return {code, out.str(), err.str()};
   }

   static void remove_files()
   {
    std::remove(input_file);
    std::remove(output_file);
    std::remove((std::string(input_file) + ".packed").c_str());
    std::remove((std::string(input_file) + ".unpacked").c_str());
    std::remove((std::string(input_file) + ".sock").c_str());
   }

   void SetUp() override
   {
    remove_files();
    File file(input_file, Open_Mode::create_new);
    Writable_Journal journal(file);
    journal.create_table("person");
    journal.add_field(Table_Id(1), "name", Type::string());
    journal.add_field(Table_Id(1), "score", Type::float64());
    journal.soft_checkpoint();
    schema_checkpoint = journal.get_checkpoint();
    journal.insert_into(Table_Id(1), Record_Id(0));
    journal.update_string(Table_Id(1), Record_Id(0), Field_Id(1), "Alice");
    journal.update_float64(Table_Id(1), Record_Id(0), Field_Id(2), 1.5);
    journal.soft_checkpoint();
    first_checkpoint = journal.get_checkpoint();
    journal.insert_into(Table_Id(1), Record_Id(2));
    journal.update_string(Table_Id(1), Record_Id(2), Field_Id(1), "Bob");
    journal.update_float64(Table_Id(1), Record_Id(2), Field_Id(2), 2.5);
    journal.update_string(Table_Id(1), Record_Id(0), Field_Id(1), "Alicia");
    journal.soft_checkpoint();
   }

   void TearDown() override
   {
    remove_files();
   }
 };

 TEST_F(Tool_Test, logdump_formats)
 {
  const auto plain = run(logdump, {input_file});
  ASSERT_EQ(plain.code, 0);
  EXPECT_TRUE(plain.err.empty());
  EXPECT_NE(plain.out.find("update person 0 name \"Alicia\"\n"), std::string::npos);

  const auto loaded = run(logdump, {"--load", input_file});
  EXPECT_EQ(loaded.code, 0);
  EXPECT_EQ(loaded.out, plain.out);
  EXPECT_TRUE(loaded.err.empty());

  const auto schema = run(logdump, {"--schema_only", input_file});
  EXPECT_EQ(schema.code, 0);
  EXPECT_EQ(schema.out,
   "create_table person\nadd_field person name string\n"
   "add_field person score float64\n\n");

  const auto raw = run(logdump, {"--raw", input_file});
  EXPECT_EQ(raw.code, 0);
  EXPECT_NE(raw.out.find("add_field 1"), std::string::npos);
  EXPECT_NE(raw.out, plain.out);

  const auto sql = run(logdump, {"--sql", input_file});
  EXPECT_EQ(sql.code, 0);
  EXPECT_NE(sql.out.find("CREATE TABLE \"person\""), std::string::npos);
  EXPECT_NE(sql.out.find("X'416c69636961'"), std::string::npos);

  const auto checkpoints = run(logdump, {"--load", "--print_checkpoint", input_file});
  EXPECT_EQ(checkpoints.code, 0);
  EXPECT_NE(checkpoints.out.find("comment \"" + std::to_string(schema_checkpoint) + "\"\n"), std::string::npos);

  const auto header = run(logdump, {"--header", input_file});
  EXPECT_EQ(header.code, 0);
  EXPECT_NE(header.out.find("format version: "), std::string::npos);
  EXPECT_NE(header.out.find("signature: \"joedb\""), std::string::npos);
 }

 TEST_F(Tool_Test, logdump_recovery)
 {
  const auto expected = run(logdump, {input_file});
  {
   File file(input_file, Open_Mode::write_existing);
   file.pwrite("xxxxx", 5, offsetof(Header, signature));
  }
  EXPECT_THROW(run(logdump, {input_file}), Exception);
  const auto recovered = run(logdump, {"--ignore_header", input_file});
  EXPECT_EQ(recovered.code, 0);
  EXPECT_EQ(recovered.out, expected.out);
  EXPECT_TRUE(recovered.err.empty());
 }

 TEST_F(Tool_Test, json_export)
 {
  const auto json = run(joedb_to_json, {input_file});
  EXPECT_EQ(json.code, 0);
  EXPECT_TRUE(json.err.empty());
  EXPECT_EQ(json.out,
   "{\n \"person\":\n {\n  \"__size\": 2,\n"
   "  \"name\": [\"Alicia\", \"Bob\"],\n"
   "  \"score\": [1.5, 2.5]\n }\n}\n");

  const auto base64 = run(joedb_to_json, {"--base64", input_file});
  EXPECT_EQ(base64.code, 0);
  EXPECT_TRUE(base64.err.empty());
  EXPECT_NE(base64.out.find("\"name\": [\"QWxpY2lh\", \"Qm9i\"]"), std::string::npos);
 }

 TEST_F(Tool_Test, json_warnings)
 {
  {
   File file(input_file, Open_Mode::write_existing);
   Writable_Journal journal(file);
   Dummy_Writable dummy;
   journal.replay_log(dummy);
   journal.update_string(Table_Id(1), Record_Id(0), Field_Id(1), std::string(1, char(0xff)));
   journal.update_float64(Table_Id(1), Record_Id(0), Field_Id(2),
    std::numeric_limits<double>::infinity());
   journal.update_float64(Table_Id(1), Record_Id(2), Field_Id(2),
    std::numeric_limits<double>::quiet_NaN());
   journal.soft_checkpoint();
  }
  const auto json = run(joedb_to_json, {input_file});
  EXPECT_EQ(json.code, 0);
  EXPECT_NE(json.err.find("a string could not be encoded"), std::string::npos);
  EXPECT_NE(json.err.find("inf or nan value encoded as 0"), std::string::npos);
  EXPECT_NE(json.out.find("\"score\": [0, 0]"), std::string::npos);

  const auto base64 = run(joedb_to_json, {"--base64", input_file});
  EXPECT_EQ(base64.code, 0);
  EXPECT_EQ(base64.err.find("a string could not be encoded"), std::string::npos);
  EXPECT_NE(base64.out.find("\"name\": [\"/w==\", \"Qm9i\"]"), std::string::npos);
 }

 TEST_F(Tool_Test, pack_and_convert_preserve_data)
 {
  const auto expected = run(joedb_to_json, {input_file});
  for (const auto tool: {Tool(pack), Tool(convert)})
  {
   const auto result = run(tool, {input_file, output_file});
   EXPECT_EQ(result.code, 0);
   EXPECT_TRUE(result.out.empty());
   EXPECT_TRUE(result.err.empty());
   const auto actual = run(joedb_to_json, {output_file});
   EXPECT_EQ(actual.code, 0);
   EXPECT_EQ(actual.out, expected.out);
   EXPECT_TRUE(actual.err.empty());
   std::remove(output_file);
  }
 }

 TEST_F(Tool_Test, convert_until_checkpoint)
 {
  const std::string until = std::to_string(first_checkpoint);
  const auto result = run(convert, {"--until", until.c_str(), input_file, output_file});
  EXPECT_EQ(result.code, 0);
  const auto json = run(joedb_to_json, {output_file});
  EXPECT_EQ(json.code, 0);
  EXPECT_EQ(json.out,
   "{\n \"person\":\n {\n  \"__size\": 1,\n"
   "  \"name\": [\"Alice\"],\n  \"score\": [1.5]\n }\n}\n");
 }

 TEST_F(Tool_Test, convert_until_after_end)
 {
  const auto result = run(convert,
   {"--until", "9223372036854775807", input_file, output_file});
  EXPECT_EQ(result.code, 0);
  EXPECT_EQ(run(joedb_to_json, {output_file}).out,
   run(joedb_to_json, {input_file}).out);
 }

 TEST_F(Tool_Test, readonly_interpreter)
 {
  const auto before = run(joedb_to_json, {input_file});
  const auto result = run(joedbi, {"--log", "none", "--read", input_file},
   "echo on\nhelp\nquit\n");
  EXPECT_EQ(result.code, 0);
  EXPECT_TRUE(result.err.empty());
  EXPECT_NE(result.out.find("General commands"), std::string::npos);
  EXPECT_EQ(run(joedb_to_json, {input_file}).out, before.out);
 }

 TEST_F(Tool_Test, write_lock_streams)
 {
  const auto result = run(lock, {input_file}, "\n");
  EXPECT_EQ(result.code, 0);
  EXPECT_EQ(result.out, std::string("Locking ") + input_file +
   "...\nLocked. Enter to stop.");
  EXPECT_TRUE(result.err.empty());
 }

 TEST_F(Tool_Test, client_database_modes)
 {
  for (const char *mode: {"interpreted", "none", "dump", "dump_tail", "sql", "sql_tail"})
  {
   SCOPED_TRACE(mode);
   const auto result = run(client, {"--log", "none", "--db", mode, "--read", input_file},
    "db\nhelp\nquit\nquit\n");
   EXPECT_EQ(result.code, 0);
   EXPECT_TRUE(result.err.empty());
   EXPECT_EQ(result.out.find("Exception caught"), std::string::npos) << result.out;
   if (std::string(mode) == "sql")
   {
    EXPECT_NE(result.out.find("CREATE TABLE \"person\""), std::string::npos);
   }
   if (std::string(mode) == "dump_tail" || std::string(mode) == "sql_tail")
   {
    EXPECT_EQ(result.out.find("create_table person"), std::string::npos);
    EXPECT_EQ(result.out.find("CREATE TABLE"), std::string::npos);
   }
  }
 }

 TEST_F(Tool_Test, client_journal_transaction)
 {
  const auto result = run(client, {"--log", "none", "--db", "none", input_file},
   "set_timestamp false\nset_valid_data false\nset_hard_checkpoint true\n"
   "transaction\ncomment \"journal transaction\"\nquit\npush\nquit\n");
  EXPECT_EQ(result.code, 0);
  EXPECT_TRUE(result.err.empty());
  EXPECT_EQ(result.out.find("Exception caught"), std::string::npos) << result.out;
  EXPECT_NE(run(logdump, {input_file}).out.find("comment \"journal transaction\"\n"), std::string::npos);
 }

 TEST_F(Tool_Test, push_to_file)
 {
  const auto result = run(push, {"--log", "none", input_file, "file", output_file});
  EXPECT_EQ(result.code, 0);
  EXPECT_TRUE(result.out.empty());
  EXPECT_TRUE(result.err.empty());
  EXPECT_EQ(run(joedb_to_json, {output_file}).out, run(joedb_to_json, {input_file}).out);

  const std::string until = std::to_string(first_checkpoint);
  const auto followed = run(push,
   {"--log", "none", "--follow", "--until", until.c_str(), input_file, "file", output_file});
  EXPECT_EQ(followed.code, 0);
  EXPECT_TRUE(followed.err.empty());
 }

 TEST_F(Tool_Test, push_until_checkpoint)
 {
  const std::string until = std::to_string(first_checkpoint);
  const auto result = run(push,
   {"--log", "none", "--until", until.c_str(), input_file, "file", output_file});
  EXPECT_EQ(result.code, 0);
  const auto json = run(joedb_to_json, {output_file});
  EXPECT_EQ(json.code, 0);
  EXPECT_NE(json.out.find("\"name\": [\"Alice\"]"), std::string::npos) << json.out;
 }

 TEST_F(Tool_Test, blob_export_and_pack_rejection)
 {
  {
   File file(input_file, Open_Mode::write_existing);
   Writable_Journal journal(file);
   Dummy_Writable dummy;
   journal.replay_log(dummy);
   journal.add_field(Table_Id(1), "payload", Type::blob());
   const Blob blob = journal.write_blob(std::string("a\0b", 3));
   journal.update_blob(Table_Id(1), Record_Id(0), Field_Id(3), blob);
   journal.soft_checkpoint();
  }
  const auto text = run(logdump, {"--blob", input_file});
  EXPECT_EQ(text.code, 0);
  EXPECT_NE(text.out.find("write_blob"), std::string::npos) << text.out;
  const auto sql = run(logdump, {"--sql", "--blob", input_file});
  EXPECT_EQ(sql.code, 0);
  EXPECT_NE(sql.out.find("X'610062'"), std::string::npos) << sql.out;

  EXPECT_THROW(run(pack, {input_file, output_file}), Exception);
 }

 TEST_F(Tool_Test, convert_reopened_journal)
 {
  {
   File file(input_file, Open_Mode::write_existing);
   Writable_Journal journal(file);
   // Replay to restore the compression context before appending.
   Dummy_Writable dummy;
   journal.replay_log(dummy);
   journal.update_string(Table_Id(1), Record_Id(0), Field_Id(1), "updated");
   journal.soft_checkpoint();
  }
  const auto expected = run(joedb_to_json, {input_file});
  ASSERT_EQ(expected.code, 0);
  ASSERT_NE(expected.out.find("\"updated\""), std::string::npos);
  Result converted{};
  ASSERT_NO_THROW(converted = run(convert, {input_file, output_file}));
  EXPECT_EQ(converted.code, 0);
  EXPECT_EQ(run(joedb_to_json, {output_file}).out, expected.out);

  File input(input_file, Open_Mode::read_existing);
  File output(output_file, Open_Mode::read_existing);
  EXPECT_EQ(output.get_size(), input.get_size());
  EXPECT_TRUE(input.equal_to(output, Header::ssize, input.get_size()));
 }

 TEST_F(Tool_Test, conversion_recovery_and_existing_output)
 {
  {
   File file(input_file, Open_Mode::write_existing);
   file.pwrite("xxxxx", 5, offsetof(Header, signature));
  }
  EXPECT_THROW(run(convert, {input_file, output_file}), Exception);
  EXPECT_EQ(run(convert, {"--ignore_header", input_file, output_file}).code, 0);
  const auto before = run(joedb_to_json, {output_file});
  EXPECT_EQ(before.code, 0);
  EXPECT_THROW(run(convert, {"--ignore_header", input_file, output_file}), Exception);
  EXPECT_EQ(run(joedb_to_json, {output_file}).out, before.out);
 }

 TEST_F(Tool_Test, invalid_client_arguments)
 {
  EXPECT_THROW(run(client, {"--log", "none", "--db", "invalid", input_file}), Exception);
  EXPECT_THROW(run(client, {"--log", "none", "server"}), Exception);
  EXPECT_THROW(run(client, {"--log", "none", input_file, "unknown_connection"}), Exception);
  EXPECT_THROW(run(logdump, {"missing_tool_input.joedb.tmp"}), Exception);
 }

 TEST(ui, json_missing_reference_table)
 {
  Database db;
  db.create_table("parent");
  db.create_table("child");
  db.add_field(Table_Id(2), "parent", Type::reference(Table_Id(1)));
  db.insert_into(Table_Id(2), Record_Id(0));
  db.update_reference(Table_Id(2), Record_Id(0), Field_Id(1), Record_Id(42));
  db.drop_table(Table_Id(1));
  std::ostringstream out;
  EXPECT_EQ(write_json(out, db, false), JSON_Error::ok);
  EXPECT_EQ(out.str(),
   "{\n \"child\":\n {\n  \"__size\": 1,\n  \"parent\": [42]\n }\n}\n");
 }


 TEST_F(Tool_Test, interpreter_include)
 {
  {
   std::ofstream script(output_file);
   script << "echo off\ncreate_table included\n";
  }
  const auto result = run(joedbi, {"--log", "none", "memory"},
   std::string("include \"") + output_file + "\"\njson\n");
  EXPECT_EQ(result.code, 0);
  EXPECT_TRUE(result.err.empty());
  EXPECT_NE(result.out.find("\"included\""), std::string::npos);
  EXPECT_EQ(result.out.find("Exception caught"), std::string::npos) << result.out;

  const auto missing = run(joedbi, {"--log", "none", "memory"},
   "include \"missing_include.joedbi.tmp\"\ncreate_table after_error\njson\n");
  EXPECT_EQ(missing.code, 0);
  EXPECT_NE(missing.out.find("error opening missing_include.joedbi.tmp"), std::string::npos);
  EXPECT_NE(missing.out.find("\"after_error\""), std::string::npos);
 }

 TEST_F(Tool_Test, include_recursion_and_recovery)
 {
  {
   std::ofstream script(output_file);
   script << "include \"" << output_file << "\"\n";
  }
  Command_Interpreter interpreter;
  interpreter.set_rethrow(true);
  std::istringstream in(std::string("include \"") + output_file + "\"\n");
  std::ostringstream out;
  try
  {
   interpreter.main_loop(in, out);
   FAIL() << "Expected the recursive include to fail";
  }
  catch (const Exception &e)
  {
   EXPECT_NE(std::string(e.what()).find("Reached max recursion depth"), std::string::npos);
  }
  {
   std::ofstream script(output_file);
   script << "echo off\n";
  }
  std::istringstream retry(std::string("include \"") + output_file + "\"\n");
  EXPECT_NO_THROW(interpreter.main_loop(retry, out));
 }

 TEST(ui, interpreter_control_commands)
 {
  Command_Interpreter interpreter;
  std::istringstream in("echo off\nprompt on\nprompt off\n?\t\r\nabout\n\t\n");
  std::ostringstream out;
  interpreter.main_loop(in, out);
  EXPECT_TRUE(interpreter.is_last_line_empty());
  EXPECT_NE(out.str().find("General commands"), std::string::npos);
  EXPECT_NE(out.str().find("joedb version:"), std::string::npos);
  EXPECT_NE(out.str().find("\x1b[36mjoedbi>"), std::string::npos);
  EXPECT_EQ(out.str().find("OK:"), std::string::npos);
 }

 TEST_F(Tool_Test, interpreter_abort)
 {
  const auto before = run(joedb_to_json, {input_file});
  EXPECT_THROW(run(joedbi, {"--log", "none", input_file},
   "update person 0 name \"uncommitted\"\nabort\n"), Exception);
  EXPECT_EQ(run(joedb_to_json, {input_file}).out, before.out);
 }

 TEST_F(Tool_Test, inplace_pack_backup)
 {
  const auto before = run(joedb_to_json, {input_file});
  const auto original_log = run(logdump, {input_file});
  ASSERT_NO_THROW(inplace_pack(input_file));
  EXPECT_EQ(run(joedb_to_json, {input_file}).out, before.out);
  const std::string backup = std::string(input_file) + ".unpacked";
  EXPECT_EQ(run(logdump, {backup.c_str()}).out, original_log.out);
  const auto packed = run(logdump, {input_file});
  EXPECT_EQ(packed.out.find("\"Alice\""), std::string::npos);
  EXPECT_NE(packed.out.find("\"Alicia\""), std::string::npos);
 }

 TEST_F(Tool_Test, incomplete_file_arguments)
 {
  const std::vector<const char *> protocols
  {
#ifdef JOEDB_HAS_SSH
   "sftp",
#endif
#ifdef JOEDB_HAS_CURL
   "curl",
#endif
#ifdef JOEDB_HAS_BROTLI
   "brotli",
#endif
  };
  for (const char *protocol: protocols)
  {
   SCOPED_TRACE(protocol);
   const auto result = run(joedbi, {"--log", "none", protocol});
   EXPECT_EQ(result.code, 1);
   EXPECT_TRUE(result.out.empty());
   EXPECT_NE(result.err.find("usage:"), std::string::npos);
  }
 }

 TEST(ui, client_reports_pulled_data)
 {
  Memory_File server_file;
  Memory_File client_file;
  Writable_Journal journal(server_file);
  Journal_Connection connection(journal);
  Writable_Database_Client client(client_file, connection);
  journal.create_table("from_server");
  journal.soft_checkpoint();
  Writable_Client_Command_Processor interpreter(client);
  std::istringstream in("pull\nquit\n");
  std::ostringstream out;
  interpreter.main_loop(in, out);
  EXPECT_NE(out.str().find("pulled "), std::string::npos);
  EXPECT_EQ(out.str().find("Exception caught"), std::string::npos) << out.str();
  EXPECT_NE(client.get_database().find_table("from_server"), Table_Id(0));
 }

#if defined(JOEDB_HAS_ASIO) && __cplusplus >= 202002L
 TEST_F(Tool_Test, server_invalid_endpoint)
 {
  const std::string endpoint = std::string(output_file) + "/server.sock";
  EXPECT_THROW(run(server,
   {"--log", "none", "--socket", endpoint.c_str(), input_file}), std::exception);
 }

 TEST_F(Tool_Test, multi_server_startup_failure_cleanup)
 {
  const std::string missing = std::string(output_file) + "/missing.joedb";
  EXPECT_THROW(run(multi_server, {"--log", "none", input_file, missing.c_str()}), std::exception);
  const std::string endpoint = std::string(input_file) + ".sock";
  EXPECT_FALSE(std::filesystem::exists(endpoint));
  EXPECT_EQ(run(joedb_to_json, {input_file}).code, 0);
 }
#endif

 TEST_F(Tool_Test, invalid_numeric_update_preserves_data)
 {
  const auto before = run(joedb_to_json, {input_file});
  const auto result = run(joedbi, {"--log", "none", input_file},
   "update person 0 score not_a_number\n");
  EXPECT_EQ(result.code, 0);
  EXPECT_NE(result.out.find("Exception caught"), std::string::npos) << result.out;
  EXPECT_EQ(run(joedb_to_json, {input_file}).out, before.out);
 }


 TEST_F(Tool_Test, invalid_updates_preserve_data)
 {
  const auto before = run(joedb_to_json, {input_file});
  for (const char *command:
   {"update person 0 score\n", "update person 0 name \"unfinished\n",
    "update person 0 name trailing\\\n"})
  {
   SCOPED_TRACE(command);
   const auto result = run(joedbi, {"--log", "none", input_file}, command);
   EXPECT_EQ(result.code, 0);
   EXPECT_NE(result.out.find("failed parsing value"), std::string::npos) << result.out;
   EXPECT_EQ(run(joedb_to_json, {input_file}).out, before.out);
  }
 }

 TEST_F(Tool_Test, invalid_vector_updates_preserve_data)
 {
  const auto inserted = run(joedbi, {"--log", "none", input_file},
   "insert_into person 1 \"Middle\" 3.5\n");
  ASSERT_EQ(inserted.code, 0);
  ASSERT_EQ(inserted.out.find("Exception caught"), std::string::npos) << inserted.out;
  const auto before = run(joedb_to_json, {input_file});
  for (const char *command:
   {"update_vector person 0 score 3 10 invalid 30\n",
    "update_vector person 0 score 3 10 20\n",
    "update_vector person 0 name 3 First Second\n"})
  {
   SCOPED_TRACE(command);
   const auto result = run(joedbi, {"--log", "none", input_file}, command);
   EXPECT_EQ(result.code, 0);
   EXPECT_NE(result.out.find("failed parsing value"), std::string::npos) << result.out;
   EXPECT_EQ(run(joedb_to_json, {input_file}).out, before.out);
  }
 }

 TEST_F(Tool_Test, valid_updates_at_end_of_line)
 {
  const auto result = run(joedbi, {"--log", "none", input_file},
   "insert_into person 1 \"Middle\" 3.5\n"
   "update_vector person 0 score 3 4 5 6\n"
   "update_vector person 0 name 3 Alpha \"\" Omega\n"
   "update person 0 score 8.25\n"
   "update person 0 name Final");
  EXPECT_EQ(result.code, 0);
  EXPECT_EQ(result.out.find("Exception caught"), std::string::npos) << result.out;
  const auto json = run(joedb_to_json, {input_file});
  EXPECT_EQ(json.out,
   "{\n \"person\":\n {\n  \"__size\": 3,\n"
   "  \"name\": [\"Final\", \"\", \"Omega\"],\n"
   "  \"score\": [8.25, 5, 6]\n }\n}\n");
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
