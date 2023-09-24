#include "joedb/db/multi_server_interpreted.h"

#include "gtest/gtest.h"

#include <fstream>

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, Interpreted_Database)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::multi_server::Interpreted_Database db
 (
  std::ifstream("multi_server.joedbi")
 );

 EXPECT_EQ(db.get_server_table().get_size(), 2);
}
