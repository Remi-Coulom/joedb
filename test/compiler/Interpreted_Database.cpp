#include "joedb/db/multi_server_readonly.h"
#include "joedb/journal/Interpreted_File.h"

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, Interpreted_Database)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::multi_server::Generic_Readonly_Database db
 (
  joedb::Interpreted_File("multi_server.joedbi")
 );

 EXPECT_EQ(db.get_server_table().get_size(), 2UL);
}
