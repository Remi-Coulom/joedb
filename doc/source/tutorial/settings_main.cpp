#include "settings/Interpreted_Database.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 settings::Interpreted_Database db("../custom_settings.joedbi");

 std::cout << "dark mode: " << db.get_dark_mode() << '\n';

 return 0;
}
