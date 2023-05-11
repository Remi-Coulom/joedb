#include "joedb/journal/File.h"

#include <iostream>
#include <iomanip>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 std::cout << std::hex;
 std::cout << std::setfill('0');

 bool fast = false;
 int arg_start = 1;

 if (argc > 1 && argv[1] == std::string("--fast"))
 {
  fast = true;
  arg_start++;
 }

 for (int arg_index = arg_start; arg_index < argc; arg_index++)
 {
  const char * const file_name = argv[arg_index];

  try
  {
   joedb::File file(file_name, joedb::Open_Mode::read_existing);

   const joedb::SHA_256::Hash hash = fast ?
    file.get_fast_hash(0, file.get_size()) :
    file.get_hash();

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
