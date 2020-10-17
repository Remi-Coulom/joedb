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
  private:
   static const DWORD desired_access[];
   static const DWORD share_mode[];
   static const DWORD creation_disposition[];

  private:
   const HANDLE file;

   void throw_last_error() const;

  protected:
   size_t read_buffer() override;
   void write_buffer() override;
   int seek(int64_t offset) override;
   void sync() override;

  public:
   Windows_File(const char *file_name, Open_Mode mode);
   Windows_File(const std::string &file_name, Open_Mode mode):
    Windows_File(file_name.c_str(), mode)
   {
   }

   int64_t get_size() const override;

   ~Windows_File() override;
 };
}

#endif
