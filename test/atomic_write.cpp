#include "joedb/journal/File.h"

int main()
{
 std::remove("atomic.bin");
 joedb::File file("atomic.bin", joedb::Open_Mode::create_new);

 constexpr size_t offset = 1;

 {
  std::string s(offset, '.');
  file.pwrite(s.data(), s.size(), 0);
 }

 const std::string s[2] =
 {
  "xxxxxxxxxxxxxxxx",
  "yyyyyyyyyyyyyyyy"
 };

 for (int i = 0; ; i ^= 1)
  file.pwrite(s[i].data(), s[i].size(), offset);

 return 0;
};
