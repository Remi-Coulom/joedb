#include "joedb/compiler/write_atomically.h"
#include "joedb/journal/fstream.h"
#include "joedb/error/Exception.h"

#include <filesystem>

namespace joedb
{
 void write_atomically
 (
  const std::string &dir_name,
  const std::string &file_name,
  const std::function<void(std::ostream &)> &write
 )
 {
  const std::string final_file_name = dir_name + "/" + file_name;
  const std::string temporary_file_name = final_file_name + ".joedbtmp." +
    std::to_string(ptrdiff_t(&final_file_name));

  std::filesystem::create_directories(dir_name);
  std::remove(temporary_file_name.c_str());

  {
   joedb::ofstream out(temporary_file_name, Open_Mode::create_new);
   write(out);
   out.flush();
  }

  if (std::rename(temporary_file_name.c_str(), final_file_name.c_str()))
   throw Exception("Error renaming to file: " + final_file_name);
 }
}
