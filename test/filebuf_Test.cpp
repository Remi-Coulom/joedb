#include "joedb/journal/filebuf.h"
#include "joedb/journal/Memory_File.h"

#include "gtest/gtest.h"

#include <ostream>
#include <sstream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 TEST(filebuf, Write_Hello)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;

  joedb::filebuf filebuf(file);
  std::ostream out(&filebuf);
  out << "Hello";
  out.flush();

  EXPECT_EQ(file.get_size(), 5);
  EXPECT_EQ(file.get_data()[0], 'H');
  EXPECT_EQ(file.get_data()[1], 'e');
  EXPECT_EQ(file.get_data()[2], 'l');
  EXPECT_EQ(file.get_data()[3], 'l');
  EXPECT_EQ(file.get_data()[4], 'o');
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(filebuf, Read_Hello_Char)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;
  file.write<char>('H');
  file.write<char>('e');
  file.write<char>('l');
  file.write<char>('l');
  file.write<char>('o');
  file.set_position(0);

  joedb::filebuf filebuf(file);
  std::istream in(&filebuf);
  EXPECT_EQ(in.get(), 'H');
  EXPECT_EQ(in.get(), 'e');
  EXPECT_EQ(in.get(), 'l');
  EXPECT_EQ(in.get(), 'l');
  EXPECT_EQ(in.get(), 'o');
  EXPECT_FALSE(in.eof());
  in.get();
  EXPECT_TRUE(in.eof());
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(filebuf, Read_Hello_String)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;
  file.write<char>('H');
  file.write<char>('e');
  file.write<char>('l');
  file.write<char>('l');
  file.write<char>('o');
  file.set_position(0);

  joedb::filebuf filebuf(file);
  std::istream in(&filebuf);
  std::string s;
  in >> s;
  EXPECT_EQ(s, "Hello");
  EXPECT_TRUE(in.eof());
 }

 ////////////////////////////////////////////////////////////////////////////
 static void stream_test(std::iostream &io)
 ////////////////////////////////////////////////////////////////////////////
 {
  io.put('a');
  io.put('b');
  io.seekg(0);
  io.seekp(0);
  io.put('X');
  EXPECT_EQ(io.get(), 'X');
  EXPECT_EQ(io.get(), 'b');
  io.seekg(1);
  EXPECT_EQ(io.get(), 'b');
  io.put('Y');
  io.seekg(1);
  EXPECT_EQ(io.get(), 'Y');
  io.seekp(0);
  io.put('Z');
  io.seekg(0);
  EXPECT_EQ(io.get(), 'Z');

  io << "Hello World!";
  io << "How are you?";
  io.seekp(6);
  io << "QSDF";
  io.flush();
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(filebuf, stringstream_ab)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::stringstream io("ab");
  stream_test(io);
  EXPECT_EQ(io.str(), "ZHelloQSDFld!How are you?");
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(filebuf, filebuf_ab)
 ////////////////////////////////////////////////////////////////////////////
 {
  Memory_File file;
  joedb::filebuf filebuf(file);
  std::iostream io(&filebuf);
  stream_test(io);
  std::string s(file.get_data().data(), file.get_data().size());
  EXPECT_EQ(s, "ZHelloQSDFld!How are you?");
 }
}
