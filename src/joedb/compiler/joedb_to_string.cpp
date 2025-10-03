#include "joedb/ui/main_wrapper.h"
#include "joedb/ui/type_io.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"

namespace joedb
{
 static int to_string(joedb::Arguments &args)
 {
  const std::string_view file_name = args.get_next("<file_name>");

  if (args.missing())
  {
   args.print_help(std::cerr);
   return 1;
  }

  File file(file_name.data(), Open_Mode::read_existing);
  Memory_File memory;
  file.copy_to(memory);

  write_string(std::cout, memory.get_data());

  return 0;
 }
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(joedb::to_string, argc, argv);
}
