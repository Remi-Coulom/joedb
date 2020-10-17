#include "joedb/journal/Portable_File.h"
#include "joedb/Exception.h"

#include <cstdio>

bool joedb::Portable_File::lock_file()
{
 return true;
}

void joedb::Portable_File::sync()
{
}

int joedb::Portable_File::seek(int64_t offset, int origin) const
{
 return std::fseek(file, long(offset), origin);
}

int64_t joedb::Portable_File::tell() const
{
 return std::ftell(file);
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Portable_File::try_open(const char *file_name, Open_Mode new_mode)
/////////////////////////////////////////////////////////////////////////////
{
 static const char *mode_string[3] = {"rb", "r+b", "w+b"};
 mode = new_mode;
 file = std::fopen(file_name, mode_string[static_cast<size_t>(mode)]);
 return file != nullptr;
}

/////////////////////////////////////////////////////////////////////////////
joedb::Portable_File::Portable_File(const char *file_name, Open_Mode new_mode)
/////////////////////////////////////////////////////////////////////////////
{
 if (new_mode == Open_Mode::write_existing_or_create_new)
 {
  try_open(file_name, Open_Mode::write_existing) ||
  try_open(file_name, Open_Mode::create_new);
 }
 else if (new_mode == Open_Mode::create_new)
 {
  if (try_open(file_name, Open_Mode::read_existing))
  {
   close_file();
   throw Exception("File already exists: " + std::string(file_name));
  }
  else
   try_open(file_name, Open_Mode::create_new);
 }
 else
  try_open(file_name, new_mode);

 if (!file)
  throw Exception("Cannot open file: " + std::string(file_name));

 if ((mode == Open_Mode::write_existing ||
      mode == Open_Mode::create_new) && !lock_file())
 {
  close_file();
  throw Exception("File locked: " + std::string(file_name));
 }

 std::setvbuf(file, nullptr, _IONBF, 0);
}

/////////////////////////////////////////////////////////////////////////////
size_t joedb::Portable_File::read_buffer()
/////////////////////////////////////////////////////////////////////////////
{
 return std::fread(buffer, 1, buffer_size, file);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Portable_File::write_buffer()
/////////////////////////////////////////////////////////////////////////////
{
 const size_t written = std::fwrite(buffer, 1, write_buffer_index, file);
 if (written != write_buffer_index)
  throw Exception("Error writing file");
}

/////////////////////////////////////////////////////////////////////////////
int joedb::Portable_File::seek(int64_t offset)
/////////////////////////////////////////////////////////////////////////////
{
 return seek(offset, SEEK_SET);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Portable_File::close_file()
/////////////////////////////////////////////////////////////////////////////
{
 if (file)
 {
  flush();
  fclose(file);
  file = nullptr;
 }
}

/////////////////////////////////////////////////////////////////////////////
int64_t joedb::Portable_File::get_size() const
/////////////////////////////////////////////////////////////////////////////
{
 const int64_t current_tell = tell();
 seek(0, SEEK_END);
 const int64_t result = tell();
 seek(current_tell, SEEK_SET);

 return result;
}

/////////////////////////////////////////////////////////////////////////////
joedb::Portable_File::~Portable_File()
/////////////////////////////////////////////////////////////////////////////
{
 close_file();
}
