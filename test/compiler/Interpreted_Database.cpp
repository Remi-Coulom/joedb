#include "joedb/db/multi_server/readonly.h"
#include "joedb/journal/Readonly_Interpreted_File.h"

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, Interpreted_Database)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::db::multi_server::Generic_Readonly_Database db
 (
  joedb::Readonly_Interpreted_File("multi_server.joedbi")
 );

 EXPECT_EQ(db.get_server_table().get_size(), 2UL);
}
