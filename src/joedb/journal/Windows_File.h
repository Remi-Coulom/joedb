#ifndef joedb_Windows_File_declared
#define joedb_Windows_File_declared

#include "joedb/journal/Buffered_File.h"

#include <windows.h>
#include <array>

namespace joedb
{
 class Windows_Handle: public Buffered_File
 {
  private:
   static constexpr size_t mode_count = static_cast<size_t>(joedb::Open_Mode::mode_count);
   static const std::array<DWORD, mode_count> desired_access;
   static const std::array<DWORD, mode_count> share_mode;
   static const std::array<DWORD, mode_count> creation_disposition;

  private:
   const HANDLE file;

   static void throw_last_error
   (
    const char *action,
    const char *file_name
   );

   enum class Lock_Operation
   {
    shared_lock,
    exclusive_lock,
    unlock
   };

   BOOL lock(Lock_Operation op, int64_t start, int64_t size);

  public:
   Windows_Handle(const char *file_name, Open_Mode mode);

   Windows_Handle(const Windows_Handle &) = delete;
   Windows_Handle &operator=(const Windows_Handle &) = delete;

   int64_t get_size() const override;
   size_t pread(char* data, size_t size, int64_t offset) const override;
   void pwrite(const char* data, size_t size, int64_t offset) override;

   void sync() override;

   void shared_lock(int64_t start, int64_t size) override;
   void exclusive_lock(int64_t start, int64_t size) override;
   void unlock(int64_t start, int64_t size) noexcept override;

   ~Windows_Handle() override;
 };

 /// @ingroup journal
 class Windows_File: public Windows_Handle
 {
  public:
   static constexpr bool lockable = true;
   static constexpr bool has_broken_posix_locking = false;

   Windows_File(const char *file_name, Open_Mode mode);
   Windows_File(const std::string &file_name, Open_Mode mode):
    Windows_File(file_name.c_str(), mode)
   {
   }
 };
}

#endif
