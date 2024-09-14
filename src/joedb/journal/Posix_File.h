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
 class Posix_FD: public Generic_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   int fd;

   int lock(int command, short type, int64_t start, int64_t size);

  protected:
   size_t pread(char *buffer, size_t size, int64_t offset) override;
   void pwrite(const char *buffer, size_t size, int64_t offset) override;
   void raw_sync() override;

  public:
   static void throw_last_error(const char *action, const char *file_name);

   Posix_FD(int fd, Open_Mode mode):
    Generic_File(mode),
    fd(fd)
   {
   }

   Posix_FD(const char *file_name, Open_Mode mode);

   Posix_FD(const Posix_FD &) = delete;
   Posix_FD &operator=(const Posix_FD &) = delete;

   int64_t get_size() const override;
   void shared_lock(int64_t start, int64_t size) override;
   bool try_exclusive_lock(int64_t start, int64_t size);
   void exclusive_lock(int64_t start, int64_t size) override;
   void unlock(int64_t start, int64_t size) override;

   ~Posix_FD() override;
 };

 ////////////////////////////////////////////////////////////////////////////
 class Posix_File: public Posix_FD
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   Posix_File(int fd, Open_Mode mode):
    Posix_FD(fd, mode)
   {
   }

   Posix_File(const char *file_name, Open_Mode mode);

   Posix_File(const std::string &file_name, Open_Mode mode):
    Posix_File(file_name.c_str(), mode)
   {
   }
 };
}

#endif
