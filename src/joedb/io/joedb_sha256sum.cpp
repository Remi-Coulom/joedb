#include "joedb/journal/SHA_256.h"
#include "joedb/journal/Generic_File.cpp"
#include "joedb/journal/File.cpp"
#include "joedb/Destructor_Logger.cpp"

#include <iostream>
#include <iomanip>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 std::cout << std::hex;
 std::cout << std::setfill('0');

 for (int i = 1; i < argc; i++)
 {
  const char * const file_name = argv[i];

  try
  {
   joedb::File file(file_name, joedb::Open_Mode::read_existing);
   joedb::SHA_256::Hash hash = file.get_hash();
   for (uint32_t i = 0; i < 8; i++)
    std::cout << std::setw(8) << hash[i];
   std::cout << "  " << file_name << '\n';
  }
  catch (const joedb::Exception &e)
  {
   std::cout << file_name << ": Error: " << e.what() << '\n';
  }
 }

 return 0;
}
