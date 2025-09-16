#include "joedb/compiler/write_source_code.h"

#include <fstream>
#include <filesystem>

namespace joedb
{
 void write_source_code
 (
  const std::string &dir_name,
  const std::string &file_name,
  const std::function<void(std::ostream &)> &write
 )
 {
  const std::string final_file_name = dir_name + "/" + file_name;
  std::filesystem::create_directories(dir_name);

  std::ofstream out(final_file_name, std::ios::binary | std::ios::trunc);
  write(out);
  out.flush();
 }
}
