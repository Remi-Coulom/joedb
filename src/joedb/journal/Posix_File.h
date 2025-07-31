#ifndef joedb_Posix_File_declared
#define joedb_Posix_File_declared

#include "joedb/journal/Buffered_File.h"

#include <fcntl.h>
#include <unistd.h>

#ifndef F_OFD_SETLK
#define JOEDB_HAS_BROKEN_POSIX_LOCKING
#endif

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Posix_FD: public Buffered_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   int fd;
   int lock(int command, short type, int64_t start, int64_t size);

  protected:
   bool try_exclusive_lock(int64_t start, int64_t size);

  public:
   static void throw_last_error(const char *action, const char *file_name);

   Posix_FD(int fd, Open_Mode mode):
    Buffered_File(mode),
    fd(fd)
   {
   }

   Posix_FD(const char *file_name, Open_Mode mode);

   Posix_FD(const Posix_FD &) = delete;
   Posix_FD &operator=(const Posix_FD &) = delete;

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

   void touch() override;

   ~Posix_FD() override;
 };

 /// @ingroup journal
 class Posix_File: public Posix_FD
 {
  public:
   static constexpr bool lockable = true;

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
