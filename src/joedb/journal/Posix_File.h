#ifndef joedb_Posix_File_declared
#define joedb_Posix_File_declared

#include "joedb/journal/Generic_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Posix_File: public Generic_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   int fd;

   static void throw_last_error(const char *action, const char *file_name);

#ifndef NDEBUG
   bool locked = false;
#endif

   bool try_exclusive_lock();

  protected:
   size_t raw_read(char *buffer, size_t size) final;
   void raw_write(const char *buffer, size_t size) final;
   int raw_seek(int64_t offset) final;
   void sync() final;

  public:
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

   Posix_File(int fd, size_t start, size_t length):
    Posix_File(fd, Open_Mode::read_existing)
   {
    set_slice(start, length);
   }

   int64_t raw_get_size() const final;
   void shared_lock() final;
   void exclusive_lock() final;
   void unlock() final;

   ~Posix_File() override;
 };
}

#endif
