#ifndef joedb_Posix_File_declared
#define joedb_Posix_File_declared

#include "joedb/journal/Abstract_File.h"

#include <fcntl.h>
#include <unistd.h>
#include <string>

#ifndef F_OFD_SETLK
#define JOEDB_HAS_BROKEN_POSIX_LOCKING
#endif

namespace joedb
{
 class Posix_FD
 {
  private:
   int fd;

  public:
   int fullsync();
   int get_fd() const {return fd;}

   Posix_FD(int fd): fd(fd) {}
   Posix_FD(const char *file_name, Open_Mode mode);

   Posix_FD(const Posix_FD &) = delete;
   Posix_FD &operator=(const Posix_FD &) = delete;

   ~Posix_FD();
 };

 /// @ingroup journal
 class Posix_File: public Abstract_File
 {
  private:
   Posix_FD posix_fd;
   std::string file_name;
   bool created_durably = false;

   int lock(int command, short type, int64_t start, int64_t size);
   bool try_exclusive_lock(int64_t start, int64_t size);
   void throw_last_error(const char *action) const;

  public:
   static constexpr bool lockable = true;
#ifdef JOEDB_HAS_BROKEN_POSIX_LOCKING
   static constexpr bool has_broken_posix_locking = true;
#else
   static constexpr bool has_broken_posix_locking = false;
#endif

   Posix_File(int fd, Open_Mode mode):
    Abstract_File(mode),
    posix_fd(fd)
   {
   }

   Posix_File(const char *file_name, Open_Mode mode);

   Posix_File(const std::string &file_name, Open_Mode mode):
    Posix_File(file_name.c_str(), mode)
   {
   }

   int64_t get_size() const override;
   size_t pread(char *buffer, size_t size, int64_t offset) const override;
   void pwrite(const char *buffer, size_t size, int64_t offset) override;

   void sync() override;
#if _POSIX_SYNCHRONIZED_IO > 0
   void datasync() override;
#endif

   void shared_lock(int64_t start, int64_t size) override;
   void exclusive_lock(int64_t start, int64_t size) override;
   void unlock(int64_t start, int64_t size) noexcept override;
 };
}

#endif
