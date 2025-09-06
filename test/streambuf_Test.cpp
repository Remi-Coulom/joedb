#include "joedb/journal/iostream.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/error/String_Logger.h"

#include "gtest/gtest.h"

#include <iostream>

//#define TEST_STD_STRINGBUF

namespace joedb
{
 TEST(streambuf, hello)
 {
  joedb::Memory_File file;

  {
   joedb::iostream ios(file);
   ios << "Hello " << 123;
   ios.flush();
  }

  EXPECT_EQ("Hello 123", file.get_data());
 }

 TEST(streambuf, destructor)
 {
  joedb::Memory_File file;

  {
   joedb::iostream ios(file);
   ios << 123;
  }

  EXPECT_EQ("joedb: destructor warning: streambuf: flushing buffer\n", String_Logger::the_logger.get_message());
 }

 TEST(streambuf, read)
 {
  joedb::Memory_File file;
  file.write_data("123", 3);
  file.flush();
  int n;
  joedb::iostream(file) >> n;
  EXPECT_EQ(123, n);
 }

 TEST(streambuf, showmanyc)
 {
  joedb::Memory_File file;
  file.write_data("123 456", 7);
  file.flush();

#ifdef TEST_STD_STRINGBUF
  std::stringbuf buf(file.get_data());
#else
  joedb::streambuf buf(file);
#endif

  std::iostream ios(&buf);

  EXPECT_EQ(buf.in_avail(), 7);
  int32_t n;
  ios >> n;
  EXPECT_EQ(123, n);
  EXPECT_EQ(buf.in_avail(), 4);
  ios >> n;
  EXPECT_EQ(456, n);
  EXPECT_TRUE(ios.eof());
  ios.clear();

  ios.seekp(3, std::ios::beg);
  ios << '0';
  ios.seekg(-7, std::ios::end);
  ios >> n;
  EXPECT_EQ(1230456, n);
  EXPECT_TRUE(ios.eof());
  ios.clear();

  ios.seekp(3, std::ios::beg);
  ios << '.';
  ios.seekg(-6, std::ios::cur);
  EXPECT_EQ(buf.in_avail(), 6);
  double x;
  ios >> x;
  EXPECT_EQ(x, 23.456);
 }

 TEST(streambuf, over_and_under_flow)
 {
  joedb::Memory_File file;

#ifdef TEST_STD_STRINGBUF
  std::stringbuf buf(file.get_data());
#else
  joedb::streambuf buf(file);
#endif

  std::iostream ios(&buf);

  std::vector<char> data(123456, 'x');
  ios.write(data.data(), data.size());
  ios.seekp(0, std::ios::beg);
  EXPECT_EQ(buf.in_avail(), 123456);
  EXPECT_EQ(ios.get(), 'x');
#ifdef TEST_STD_STRINGBUF
  EXPECT_EQ(buf.in_avail(), 123455);
#else
  EXPECT_EQ(buf.in_avail(), 8191);
#endif
  for (int i = 9999; --i >= 0;)
   EXPECT_EQ(ios.get(), 'x');
  ios.seekp(1, std::ios::cur);
  for (int i = 9999; --i >= 0;)
   ios.put('y');
  ios.seekg(-10000, std::ios::cur);
  EXPECT_EQ(ios.get(), 'x');
  for (int i = 9999; --i >= 0;)
   EXPECT_EQ(ios.get(), 'y');
  EXPECT_EQ(ios.get(), 'x');

  std::string buffer(10000, 0);
  ios.seekg(0, std::ios::beg);
  ios.read(buffer.data(), 3);
  EXPECT_EQ(buffer[0], 'x');
  EXPECT_EQ(buffer[1], 'y');
  EXPECT_EQ(buffer[2], 'y');
  ios.read(buffer.data(), 3);
  EXPECT_EQ(buffer[0], 'y');
  EXPECT_EQ(buffer[1], 'y');
  EXPECT_EQ(buffer[2], 'y');
  ios.read(buffer.data(), buffer.size());
  EXPECT_EQ(buffer[0], 'y');
  EXPECT_EQ(buffer[9999], 'x');

  ios.seekg(0, std::ios::beg);
  ios.read(buffer.data(), buffer.size());
  ios.seekp(1, std::ios::beg);
  ios.write(buffer.data(), buffer.size());
  ios.seekg(0, std::ios::beg);
  ios.read(buffer.data(), 4);
  EXPECT_EQ(buffer[0], 'x');
  EXPECT_EQ(buffer[1], 'x');
  EXPECT_EQ(buffer[2], 'y');
  EXPECT_EQ(buffer[3], 'y');
 }

 TEST(streambuf, unknown_size)
 {
  joedb::Abstract_File file;
  joedb::streambuf buf(file);
  EXPECT_EQ(-1, buf.pubseekoff(0, std::ios::end));
 }

 TEST(streambuf, pbackfail)
 {
  joedb::Memory_File file;

#ifdef TEST_STD_STRINGBUF
  std::stringbuf buf(file.get_data());
#else
  joedb::streambuf buf(file);
#endif

  buf.sputc('a');
  buf.sputc('b');
  buf.sputc('c');

  buf.pubsync();

  EXPECT_EQ(buf.sbumpc(), 'a');
  EXPECT_EQ(buf.sbumpc(), 'b');
  EXPECT_EQ(buf.sbumpc(), 'c');

  EXPECT_EQ(buf.sputbackc('z'), 'z');
  EXPECT_EQ(buf.sputbackc('y'), 'y');
  EXPECT_EQ(buf.sputbackc('x'), 'x');
  EXPECT_EQ(buf.sputbackc('w'), std::char_traits<char>::eof());

  EXPECT_EQ(buf.sbumpc(), 'x');
  EXPECT_EQ(buf.sbumpc(), 'y');
  EXPECT_EQ(buf.sbumpc(), 'z');

  buf.pubsync();

  EXPECT_EQ(buf.sputbackc('c'), 'c');
  EXPECT_EQ(buf.sputbackc('b'), 'b');
  EXPECT_EQ(buf.sputbackc('a'), 'a');
  EXPECT_EQ(buf.sputbackc('z'), std::char_traits<char>::eof());

  EXPECT_EQ(buf.sbumpc(), 'a');
  EXPECT_EQ(buf.sbumpc(), 'b');
  EXPECT_EQ(buf.sbumpc(), 'c');
  EXPECT_EQ(buf.sbumpc(), std::char_traits<char>::eof());
 }
}
