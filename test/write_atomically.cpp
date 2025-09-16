#include "joedb/compiler/write_atomically.h"
#include "joedb/ui/main_wrapper.cpp"

#include <thread>

namespace joedb
{
 static int main(Arguments &args)
 {
  const std::string_view arg = args.get_next("<string>");

  if (args.missing())
  {
   args.print_help(std::cerr) << '\n';
   return 1;
  }

  joedb::write_atomically(".", "atomic.txt", [&arg](std::ostream &out)
  {
   for (int i = 10; --i >= 0;)
   {
    out << arg << '\n';
    out.flush();
    std::cerr << '.';
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }
   std::cerr << '\n';
  });

  return 0;
 }
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(joedb::main, argc, argv);
}
