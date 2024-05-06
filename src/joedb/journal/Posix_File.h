#ifndef joedb_Posix_File_declared
#define joedb_Posix_File_declared

#include "joedb/journal/Generic_File.h"

#include <fcntl.h>

#ifndef F_OFD_SETLK
#define JOEDB_HAS_BRAINDEAD_POSIX_LOCKING
#endif

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Posix_File: public Generic_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   int fd;

   int lock(int command, short type, int64_t start, int64_t size);

  protected:
   size_t pread(char *buffer, size_t size, int64_t offset) final;
   void pwrite(const char *buffer, size_t size, int64_t offset) final;
   void raw_sync() final;

  public:
   static void throw_last_error(const char *action, const char *file_name);

   Posix_File(int fd, Open_Mode mode):
    Generic_File(mode),
    fd(fd)
   {
   }

   Posix_File(const char *file_name, Open_Mode mode);

   Posix_File(const std::string &file_name, Open_Mode mode):
    Posix_File(file_name.c_str(), mode)
   {
   }

   int64_t get_size() const final;
   void shared_lock(int64_t start, int64_t size) final;
   bool try_exclusive_lock(int64_t start, int64_t size);
   void exclusive_lock(int64_t start, int64_t size) final;
   void unlock(int64_t start, int64_t size) final;

   ~Posix_File() override;
 };
}

#endif
