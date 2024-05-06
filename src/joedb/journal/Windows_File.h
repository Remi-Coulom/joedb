#ifndef joedb_Windows_File_declared
#define joedb_Windows_File_declared

#include "joedb/journal/Generic_File.h"

#include <windows.h>
#include <array>

namespace joedb
{
 ///////////////////////////////////////////////////////////////////////////
 class Windows_File: public Generic_File
 ///////////////////////////////////////////////////////////////////////////
 {
  private:
   static constexpr size_t modes = static_cast<size_t>(joedb::Open_Mode::modes);
   static const std::array<DWORD, modes> desired_access;
   static const std::array<DWORD, modes> share_mode;
   static const std::array<DWORD, modes> creation_disposition;

  private:
   const HANDLE file;

   void throw_last_error
   (
    const char *action,
    const char *file_name
   ) const;

   enum class Lock_Operation
   {
    shared_lock,
    exclusive_lock,
    unlock
   };

   BOOL lock(Lock_Operation op, int64_t start, int64_t size);

  protected:
   size_t pread(char* data, size_t size, int64_t offset) final;
   void pwrite(const char* data, size_t size, int64_t offset) final;
   int64_t get_size() const final;
   void raw_sync() final;

  public:
   Windows_File(const char *file_name, Open_Mode mode);

   Windows_File(const std::string &file_name, Open_Mode mode):
    Windows_File(file_name.c_str(), mode)
   {
   }

   void shared_lock(int64_t start, int64_t size) final;
   void exclusive_lock(int64_t start, int64_t size) final;
   void unlock(int64_t start, int64_t size) final;

   ~Windows_File() override;
 };
}

#endif
