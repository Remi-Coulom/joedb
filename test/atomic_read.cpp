#include "joedb/journal/File.h"

#include <set>
#include <iostream>

int main()
{
 joedb::File file("atomic.bin", joedb::Open_Mode::read_existing);

 std::set<std::string> set;
 constexpr size_t buffer_size = 64;
 char buffer[buffer_size];

 while (true)
 {
  size_t size = file.pread(buffer, buffer_size, 0);
  std::string s(buffer, size);
  const auto it = set.find(s);
  if (it == set.end())
  {
   set.insert(s);
   std::cout << s << '\n';
  }
 }

 return 0;
}
