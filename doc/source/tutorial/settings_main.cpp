#include "settings_readonly.h"
#include "joedb/journal/Interpreted_File.h"

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 settings::Readonly_Database db
 (
  joedb::Interpreted_File("../custom_settings.joedbi")
 );

 std::cout << "dark mode: ";
 std::cout << db.get_dark_mode(db.get_Settings_table().first());
 std::cout << '\n';
 return 0;
}
