#include "../../doc/source/tutorial/settings.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 TEST(Compiler, single_row)
 ////////////////////////////////////////////////////////////////////////////
 {
  joedb::Memory_File file;

  {
   settings::Generic_File_Database db(file);

   EXPECT_EQ(db.get_dark_mode(), true);
   EXPECT_EQ(db.get_host(), "www.kayufu.com");
   EXPECT_EQ(db.get_user(), "joe");

   db.set_host(db.the_settings(), "new.host.com");
   db.checkpoint();

   // compile-time errors:
   // db.new_settings();
   // db.delete_settings(db.the_settings());
  }

  {
   const settings::Readonly_Database db(file);
   EXPECT_EQ(db.get_host(db.the_settings()), "new.host.com");
  }

  {
   joedb::Connection connection;
   settings::Client client(connection, file);
   EXPECT_EQ(client.get_database().get_host(), "new.host.com");
  }

  {
   joedb::Memory_File new_file;
   joedb::Connection connection;
   settings::Client client(connection, new_file);
   EXPECT_EQ(client.get_database().get_host(), "www.kayufu.com");
  }
 }
}