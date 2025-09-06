#include "joedb/journal/iostream.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/error/String_Logger.h"

#include "gtest/gtest.h"

#include <iostream>

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
}
