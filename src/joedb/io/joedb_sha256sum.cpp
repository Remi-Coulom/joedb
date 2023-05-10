#include "joedb/journal/File.h"

#include <iostream>
#include <iomanip>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 std::cout << std::hex;
 std::cout << std::setfill('0');

 for (int arg_index = 1; arg_index < argc; arg_index++)
 {
  const char * const file_name = argv[arg_index];

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
