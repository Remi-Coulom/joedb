#include "settings/Interpreted_File_Database.h"
#include "settings/Readonly_Interpreted_File_Database.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 const char * const file_name = "../custom_settings.joedbi";

 {
  settings::Interpreted_File_Database db(file_name);
  db.set_user("toto");
  db.soft_checkpoint();
 }

 {
  settings::Readonly_Interpreted_File_Database db(file_name);
  std::cout << "dark mode: " << db.get_dark_mode() << '\n';
  std::cout << "user: " << db.get_user() << '\n';
  std::cout << "host: " << db.get_host() << '\n';
 }

 return 0;
}
