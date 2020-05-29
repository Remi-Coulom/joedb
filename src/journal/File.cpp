#include "joedb/File.h"
#include "joedb/Exception.h"

/////////////////////////////////////////////////////////////////////////////
// System-specific functions
/////////////////////////////////////////////////////////////////////////////

#ifdef JOEDB_PORTABLE
/////////////////////////////////////////////////////////////////////////////

#include <cstdio>

bool joedb::File::lock_file()
{
 return true;
}

void joedb::File::sync()
{
}

int joedb::File::seek(int64_t offset, int origin) const
{
 return std::fseek(file, long(offset), origin);
}

int64_t joedb::File::tell() const
{
 return std::ftell(file);
}

#elif defined(_WIN32)
/////////////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include <io.h>
#include <stdio.h>
#include <FileAPI.h>
#include <corecrt_io.h>

bool joedb::File::lock_file()
{
 HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(file));
 return LockFile(hFile, 0, 0, 1, 0) == TRUE;
 // Note: this prevents another process from reading
 // -> Will fail with confusing "does not start by joedb" error
 // -> Should produce an error message if it happens
 // TODO: properly handle file sharing in Windows
}

void joedb::File::sync()
{
 _commit(_fileno(file));
}

int joedb::File::seek(int64_t offset, int origin) const
{
 return _fseeki64(file, offset, origin);
}

int64_t joedb::File::tell() const
{
 return _ftelli64(file);
}

#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
/////////////////////////////////////////////////////////////////////////////

#include <sys/file.h>
#include <unistd.h>
#include <stdio.h>

bool joedb::File::lock_file()
{
 return flock(fileno(file), LOCK_EX | LOCK_NB) == 0;
}

void joedb::File::sync()
{
 fsync(fileno(file));
}

int joedb::File::seek(int64_t offset, int origin) const
{
 return fseeko(file, off_t(offset), origin);
}

int64_t joedb::File::tell() const
{
 return ftello(file);
}

#else
/////////////////////////////////////////////////////////////////////////////

#error("Error: Unknown system. Suggestion: try defining JOEDB_PORTABLE.")

#endif

/////////////////////////////////////////////////////////////////////////////
bool joedb::File::try_open(const char *file_name, Open_Mode new_mode)
/////////////////////////////////////////////////////////////////////////////
{
 static const char *mode_string[3] = {"rb", "r+b", "w+b"};
 mode = new_mode;
 file = std::fopen(file_name, mode_string[static_cast<size_t>(mode)]);
 return file != nullptr;
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

 std::setvbuf(file, nullptr, _IONBF, 0);
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
int joedb::File::seek(int64_t offset)
/////////////////////////////////////////////////////////////////////////////
{
 return seek(offset, SEEK_SET);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::File::close_file()
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
int64_t joedb::File::get_size() const
/////////////////////////////////////////////////////////////////////////////
{
 const int64_t current_tell = tell();
 seek(0, SEEK_END);
 const int64_t result = tell();
 seek(current_tell, SEEK_SET);

 return result;
}

/////////////////////////////////////////////////////////////////////////////
joedb::File::~File()
/////////////////////////////////////////////////////////////////////////////
{
 close_file();
}
