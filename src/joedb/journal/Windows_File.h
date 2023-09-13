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

  protected:
   size_t raw_read(char *buffer, size_t size) final;
   void raw_write(const char *buffer, size_t size) final;
   int raw_seek(int64_t offset) final;
   int64_t raw_get_size() const final;
   void sync() final;

  public:
   Windows_File(const char *file_name, Open_Mode mode);

   Windows_File(const std::string &file_name, Open_Mode mode):
    Windows_File(file_name.c_str(), mode)
   {
   }

   void shared_lock() final;
   void exclusive_lock() final;
   void unlock() final;

   ~Windows_File() override;
 };
}

#endif
