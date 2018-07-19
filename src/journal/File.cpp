#include "File.h"
#include "Exception.h"

/////////////////////////////////////////////////////////////////////////////
// System-specific functions
/////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#include <stdio.h>
#include <FileAPI.h>
#include <corecrt_io.h>

bool joedb::File::lock_file()
{
 HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(file));
 return LockFile(hFile, 0, 0, 1, 0) == TRUE;
}

void joedb::File::sync()
{
 _commit(_fileno(file));
}

#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <sys/file.h>
#include <unistd.h>

bool joedb::File::lock_file()
{
 return flock(fileno(file), LOCK_EX | LOCK_NB) == 0;
}

void joedb::File::sync()
{
 fsync(fileno(file));
}

#else
#pragma message("warning: unknown system: no lock, no sync")

bool joedb::File::lock_file()
{
 return true;
}

void joedb::File::sync()
{
}

#endif

/////////////////////////////////////////////////////////////////////////////
bool joedb::File::try_open(const char *file_name, Open_Mode new_mode)
/////////////////////////////////////////////////////////////////////////////
{
 static const char *mode_string[3] = {"rb", "r+b", "w+b"};
 mode = new_mode;
 file = std::fopen(file_name, mode_string[static_cast<size_t>(mode)]);
 return file != 0;
}

/////////////////////////////////////////////////////////////////////////////
joedb::File::File(const char *file_name, Open_Mode new_mode)
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

 std::setvbuf(file, 0, _IONBF, 0);
}

/////////////////////////////////////////////////////////////////////////////
size_t joedb::File::read_buffer()
/////////////////////////////////////////////////////////////////////////////
{
 return std::fread(buffer, 1, buffer_size, file);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::File::write_buffer()
/////////////////////////////////////////////////////////////////////////////
{
 const size_t written = std::fwrite(buffer, 1, write_buffer_index, file);
 if (written != write_buffer_index)
  throw Exception("Error writing file");
}

/////////////////////////////////////////////////////////////////////////////
int joedb::File::seek(size_t offset)
/////////////////////////////////////////////////////////////////////////////
{
 return std::fseek(file, long(offset), SEEK_SET);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::File::close_file()
/////////////////////////////////////////////////////////////////////////////
{
 if (file)
 {
  flush();
  fclose(file);
  file = 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int64_t joedb::File::get_size() const
/////////////////////////////////////////////////////////////////////////////
{
 const long current_tell = std::ftell(file);
 std::fseek(file, 0, SEEK_END);
 const int64_t result = std::ftell(file);
 std::fseek(file, current_tell, SEEK_SET);
 return result;
}

/////////////////////////////////////////////////////////////////////////////
joedb::File::~File()
/////////////////////////////////////////////////////////////////////////////
{
 close_file();
}
