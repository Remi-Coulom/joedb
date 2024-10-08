#ifndef joedb_SFTP_File_declared
#define joedb_SFTP_File_declared

#include "joedb/journal/Generic_File.h"
#include "joedb/ssh/SFTP.h"

#include <fcntl.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class SFTP_File: public Generic_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   ssh::SFTP &sftp;
   const sftp_file file;

   void throw_last_error(const char *action, const char *file_name)
   {
    sftp.throw_error
    (
     (std::string(action) + ' ' + std::string(file_name) + ": ").c_str()
    );
   }

  protected:
   size_t raw_read(char *buffer, size_t size) override
   {
    const ssize_t result = sftp_read(file, buffer, size);

    if (result < 0)
     throw_last_error("reading", "sftp file");

    return size_t(result);
   }

   void raw_write(const char *buffer, size_t size) override
   {
   }

   void raw_seek(int64_t offset) override
   {
    if (sftp_seek64(file, uint64_t(offset)) < 0)
     throw_last_error("seeking in", "sftp file");
   }

  public:
   SFTP_File(ssh::SFTP &sftp, const char *file_name):
    Generic_File(Open_Mode::read_existing),
    sftp(sftp),
    file(sftp_open(sftp.get(), file_name, O_RDONLY, 0))
   {
    if (!file)
     throw_last_error("opening", file_name);
   }

   SFTP_File(const SFTP_File &) = delete;
   SFTP_File& operator=(const SFTP_File &) = delete;

   ~SFTP_File() override
   {
    sftp_close(file);
   }
 };
}

#endif
