#include "settings/Interpreted_File_Database.h"
#include "settings/Readonly_Interpreted_File_Database.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 {
  settings::Interpreted_File_Database db("../custom_settings.joedbi");
  db.set_user(db.the_settings(), "toto");
  db.soft_checkpoint();
 }

 {
  settings::Readonly_Interpreted_File_Database db("../custom_settings.joedbi");
  std::cout << "dark mode: " << db.get_dark_mode() << '\n';
  std::cout << "user: " << db.get_user() << '\n';
  std::cout << "host: " << db.get_host() << '\n';
 }

 return 0;
}
