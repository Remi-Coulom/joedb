#include "joedb/journal/Memory_File.h"

namespace joedb
{
 Plain_Memory_File_Data::Plain_Memory_File_Data(std::vector<char> &data) {}
 Plain_Memory_File_Data::Plain_Memory_File_Data() = default;
 Plain_Memory_File_Data::~Plain_Memory_File_Data() = default;
 template class Memory_File_Template<Plain_Memory_File_Data>;
}
