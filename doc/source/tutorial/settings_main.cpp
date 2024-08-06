#include "settings.h"
#include "joedb/journal/Interpreted_File.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Interpreted_File settings_file("../custom_settings.joedbi");
 settings::Generic_File_Database db(settings_file);

 std::cout << "dark mode: ";
 std::cout << db.get_dark_mode(db.get_settings_table().first());
 std::cout << '\n';
 return 0;
}
