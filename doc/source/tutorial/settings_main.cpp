#include "settings.h"
#include "joedb/journal/Interpreted_File.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Interpreted_File settings_file("../custom_settings.joedbi");
 settings::Generic_File_Database db(settings_file);

 std::cout << "dark mode: " << db.get_dark_mode() << '\n';

 return 0;
}
