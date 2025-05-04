#include "../../doc/source/tutorial/src/settings/Readonly_Interpreted_File_Database.h"

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, Interpreted_Database)
/////////////////////////////////////////////////////////////////////////////
{
 EXPECT_ANY_THROW
 (
  joedb::Readonly_Interpreted_File("this_file_does_not_exist.joedbi")
 );

 settings::Readonly_Interpreted_File_Database db
 (
  "../doc/source/tutorial/custom_settings.joedbi"
 );

 EXPECT_FALSE(db.get_dark_mode());
 EXPECT_EQ(db.get_user(), "joe");
 EXPECT_EQ(db.get_host(), "www.joedb.org");
}
