#include "settings_interpreted.h"

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 settings::Interpreted_Database db("../settings_data.joedbi");
 std::cout << "dark mode: ";
 std::cout << db.get_dark_mode(db.get_Settings_table().first());
 std::cout << '\n';
 return 0;
}
