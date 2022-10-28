#include "joedb/journal/File.h"
#include "joedb/io/exception_catcher.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////
 {
  if (argc < 2)
  {
   std::cerr << "usage: " << argv[0] << " <file_name>\n";
   return 1;
  }

  const char * const file_name = argv[1];

  std::cout << "Locking " << file_name << "...";
  std::cout.flush();

  File lock(file_name, Open_Mode::write_lock);

  std::cout << "\nLocked. Enter to stop.";
  std::cout.flush();
  std::cin.get();

  return 0;
 }
}

//////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
//////////////////////////////////////////////////////////////////
{
 joedb::exception_catcher(joedb::main, argc, argv);
}
