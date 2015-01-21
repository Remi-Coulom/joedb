#include "File.h"

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
File::File(const char *file_name, mode_t mode):
 mode(mode)
{
 static const char *mode_string[3] = {"rb", "r+b", "w+b"};
 file = std::fopen(file_name, mode_string[static_cast<int>(mode)]);
}

/////////////////////////////////////////////////////////////////////////////
void File::set_position(int64_t position)
{
 std::fseek(file, long(position), SEEK_SET);
}

/////////////////////////////////////////////////////////////////////////////
int64_t File::get_position() const
{
 return int64_t(std::ftell(file));
}

/////////////////////////////////////////////////////////////////////////////
void File::write_string(const std::string &s)
{
 write<uint64_t>(uint64_t(s.size()));
 std::fwrite(&s[0], 1, s.size(), file);
}

/////////////////////////////////////////////////////////////////////////////
void File::read_string(std::string &s)
{
 const uint64_t size = read<uint64_t>();
 s.resize(size_t(size));
 const size_t result = std::fread(&s[0], 1, size, file);
 if (result < size)
  s.resize(result);
}

/////////////////////////////////////////////////////////////////////////////
void File::flush()
{
 fflush(file);
}

/////////////////////////////////////////////////////////////////////////////
File::~File()
{
 if (file)
  fclose(file);
}
