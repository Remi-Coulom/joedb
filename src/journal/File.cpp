#include "File.h"

/////////////////////////////////////////////////////////////////////////////
// System-specific file locking
/////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <io.h>
#include <stdio.h>
bool joedb::File::lock_file()
{
 HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(file));
 return LockFile(hFile, 0, 0, 1, 0);
}
#else
#include <sys/file.h>
bool joedb::File::lock_file()
{
 return flock(fileno(file), LOCK_EX | LOCK_NB) == 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////
void joedb::File::open(const char *file_name, mode_t new_mode)
{
 write_buffer_index = 0;
 reset_read_buffer();
 position = 0;

 mode = new_mode;
 static const char *mode_string[3] = {"rb", "r+b", "w+b"};
 file = std::fopen(file_name, mode_string[static_cast<size_t>(mode)]);

 if (file)
 {
  if (lock_file())
  {
   std::setvbuf(file, 0, _IONBF, 0);
   status = status_t::success;
  }
  else
  {
   std::fclose(file);
   file = 0;
   status = status_t::locked;
  }
 }
 else
  status = status_t::failure;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::File::set_position(uint64_t new_position)
{
 flush();
 if (!std::fseek(file, long(new_position), SEEK_SET))
 {
  position = new_position;
  reset_read_buffer();
 }
}

/////////////////////////////////////////////////////////////////////////////
void joedb::File::write_string(const std::string &s)
{
 compact_write<size_t>(s.size());
 for (char c: s)
  write<char>(c);
}

/////////////////////////////////////////////////////////////////////////////
std::string joedb::File::read_string()
{
 std::string s;
 size_t size = compact_read<size_t>();
 s.resize(size);
 for (size_t i = 0; i < size; i++)
  s[i] = char(getc());
 return s;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::File::flush()
{
 if (write_buffer_index)
  flush_write_buffer();
}

/////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

void joedb::File::commit()
{
 flush();
#ifdef WIN32
 _commit(_fileno(file));
#else
 fsync(fileno(file));
#endif
}

/////////////////////////////////////////////////////////////////////////////
joedb::File::~File()
{
 flush();
 if (file)
  fclose(file);
}
