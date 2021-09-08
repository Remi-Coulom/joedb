#ifndef joedb_Windows_File_declared
#define joedb_Windows_File_declared

#include "joedb/journal/Generic_File.h"

#include <windows.h>

namespace joedb
{
 ///////////////////////////////////////////////////////////////////////////
 class Windows_File: public Generic_File
 ///////////////////////////////////////////////////////////////////////////
 {
  template<typename File_Type> friend class Local_Connection;

  private:
   static const DWORD desired_access[];
   static const DWORD share_mode[];
   static const DWORD creation_disposition[];

  private:
   const HANDLE file;

   void throw_last_error
   (
    const char *action,
    const char *file_name
   ) const;

   void lock();
   void unlock();

  protected:
   size_t raw_read(char *buffer, size_t size)  override;
   void raw_write(const char *buffer, size_t size) override;
   int raw_seek(int64_t offset) override;
   int64_t raw_get_size() const override;
   void sync() override;

  public:
   Windows_File(const char *file_name, Open_Mode mode);

   Windows_File(const std::string &file_name, Open_Mode mode):
    Windows_File(file_name.c_str(), mode)
   {
   }

   ~Windows_File() override;
 };
}

#endif
