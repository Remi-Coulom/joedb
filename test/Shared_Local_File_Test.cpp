#include "joedb/concurrency/Shared_Local_File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/concurrency/Embedded_Connection.h"

#include "gtest/gtest.h"

#include <cstdio>

/////////////////////////////////////////////////////////////////////////////
TEST(Shared_Local_File, basic)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 joedb::Embedded_Connection connection(file); 

 const char * const file_name = "shared.joedb";

 std::remove(file_name);

 {
  joedb::Shared_Local_File shared_file1(connection, file_name);
  joedb::Generic_File &file1(shared_file1.get_file());
  file1.write<int>(1234);
  file1.flush();

  joedb::Shared_Local_File shared_file2(connection, file_name);
  joedb::Generic_File &file2(shared_file2.get_file());
  EXPECT_EQ(file2.read<int>(), 1234);
  file2.set_position(file2.get_position());
  file2.write<int>(5678);
 }

 std::remove(file_name);

 joedb::Shared_Local_File shared_file3(connection, "");
}
