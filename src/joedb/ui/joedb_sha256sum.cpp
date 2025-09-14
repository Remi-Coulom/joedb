#include "joedb/journal/File.h"
#include "joedb/journal/File_Hasher.h"
#include "joedb/error/Exception.h"
#include "joedb/ui/Arguments.h"

#include <iostream>
#include <iomanip>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 std::cout << std::hex;
 std::cout << std::setfill('0');

 joedb::Arguments arguments(argc, argv);
 const bool fast = arguments.has_flag("fast");

 while (arguments.get_remaining_count())
 {
  const std::string_view file_name = arguments.get_next();

  try
  {
   joedb::File file(file_name.data(), joedb::Open_Mode::read_existing);

   const joedb::SHA_256::Hash hash = fast ?
    joedb::File_Hasher::get_fast_hash(file, 0, file.get_size()) :
    joedb::File_Hasher::get_hash(file);

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
