#ifndef joedb_SFTP_File_declared
#define joedb_SFTP_File_declared

#include "joedb/journal/Buffered_File.h"
#include "joedb/ssh/SFTP.h"

#include <fcntl.h>

namespace joedb
{
 /// @ingroup journal
 class SFTP_File: public Buffered_File
 {
  private:
   ssh::SFTP &sftp;
   const sftp_file file;

   void throw_last_error(const char *action, const char *file_name) const
   {
    sftp.throw_error
    (
     (std::string(action) + ' ' + std::string(file_name) + ": ").c_str()
    );
   }

   mutable int64_t pos;

   void seek(int64_t offset) const
   {
    if (offset == pos)
     return;

    if (sftp_seek64(file, uint64_t(offset)) < 0)
     throw_last_error("seeking in", "sftp file");

    pos = offset;
   }

  public:
   SFTP_File(ssh::SFTP &sftp, const char *file_name):
    Buffered_File(Open_Mode::read_existing),
    sftp(sftp),
    file(sftp_open(sftp.get(), file_name, O_RDONLY, 0))
   {
    if (!file)
     throw_last_error("opening", file_name);
   }

   SFTP_File(const SFTP_File &) = delete;
   SFTP_File& operator=(const SFTP_File &) = delete;

   size_t pread(char *data, size_t size, int64_t offset) const override
   {
    seek(offset);

    const ssize_t result = sftp_read(file, data, size);

    if (result < 0)
     throw_last_error("reading", "sftp file");

    pos += result;
    return size_t(result);
   }

   ~SFTP_File() override
   {
    sftp_close(file);
   }
 };
}

#endif
