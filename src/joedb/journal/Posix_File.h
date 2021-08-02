#ifndef joedb_Posix_File_declared
#define joedb_Posix_File_declared

#include "joedb/journal/Generic_File.h"

namespace joedb
{
 ///////////////////////////////////////////////////////////////////////////
 class Posix_File: public Generic_File
 ///////////////////////////////////////////////////////////////////////////
 {
  private:
   int fd;

   void throw_last_error(const char *action, const char *file_name) const;

  protected:
   size_t raw_read(char *buffer, size_t size) override;
   void raw_write(const char *buffer, size_t size) override;
   int raw_seek(int64_t offset) override;
   void sync() override;

  public:
   Posix_File(const char *file_name, Open_Mode mode);
   Posix_File(const std::string &file_name, Open_Mode mode):
    Posix_File(file_name.c_str(), mode)
   {
   }

   int64_t raw_get_size() const override;

   ~Posix_File() override;
 };
}

#endif
