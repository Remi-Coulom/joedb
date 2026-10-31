#include "joedb/ui/diagnostics.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/Memory_File.h"

#include "gtest/gtest.h"

#include <sstream>
#include "joedb/error/Stream_Logger.h"
#include "joedb/error/Global_Logger.h"
#include "String_Logger.h"
#include <thread>
#include <set>
#include <vector>

/////////////////////////////////////////////////////////////////////////////
TEST(diagnostics, about)
/////////////////////////////////////////////////////////////////////////////
{
 std::stringstream out;
 joedb::about_joedb(out);
}

/////////////////////////////////////////////////////////////////////////////
TEST(diagnostics, dump_header)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 {
  joedb::Writable_Journal journal(file);
 }
 std::stringstream out;
 joedb::dump_header(out, file);
}

TEST(Logger, stream_messages_and_failed_flush)
{
 std::ostringstream out;
 joedb::Stream_Logger tagged(out, "test");
 tagged.log("message");
 EXPECT_NE(out.str().find("test: message\n"), std::string::npos);
 std::ostringstream plain;
 joedb::Stream_Logger untagged(plain);
 untagged.log("plain message");
 EXPECT_NE(plain.str().find("plain message\n"), std::string::npos);

 class Failed_Flush: public std::stringbuf
 {
  int sync() override {return -1;}
 } buffer;
 std::ostream failed(&buffer);
 failed.exceptions(std::ios::badbit);
 joedb::Stream_Logger logger(failed);
 EXPECT_NO_THROW(logger.log("failure is contained"));
 EXPECT_TRUE(failed.bad());
}

TEST(Logger, global_logger_lifecycle)
{
 joedb::String_Logger logger;
 joedb::Global_Logger::set_logger(&logger);
 joedb::Global_Logger::warning("captured");
 EXPECT_EQ(logger.get_message(), "captured");
 joedb::Global_Logger::remove_logger();
 joedb::Global_Logger::warning("ignored");
 EXPECT_EQ(logger.get_message(), "captured");
 joedb::Global_Logger::set_logger();
 joedb::Global_Logger::set_logger(&joedb::String_Logger::the_logger);
}

TEST(Logger, concurrent_messages_remain_whole)
{
 std::ostringstream out;
 joedb::Stream_Logger logger(out, "parallel");
 std::vector<std::thread> threads;
 for (int worker = 0; worker < 4; ++worker)
 {
  threads.emplace_back([&, worker]()
  {
   for (int i = 0; i < 50; ++i)
    logger.log(std::to_string(worker) + ":" + std::to_string(i));
  });
 }
 for (auto &thread: threads)
  thread.join();
 std::istringstream lines(out.str());
 std::set<std::string> messages;
 std::string line;
 while (std::getline(lines, line))
 {
  const size_t marker = line.find("parallel: ");
  ASSERT_NE(marker, std::string::npos) << line;
  messages.insert(line.substr(marker + 10));
 }
 ASSERT_EQ(messages.size(), 200);
 for (int worker = 0; worker < 4; ++worker)
  for (int i = 0; i < 50; ++i)
   EXPECT_EQ(messages.count(std::to_string(worker) + ":" + std::to_string(i)), 1);
}
