#ifndef joedb_ssh_SFTP_File_declared
#define joedb_ssh_SFTP_File_declared

#include "joedb/ssh/SFTP.h"

namespace joedb
{
 namespace ssh
 {
  ////////////////////////////////////////////////////////////////////////////
  class SFTP_File
  ////////////////////////////////////////////////////////////////////////////
  {
   private:
    const sftp_file file;

   public:
    SFTP_File(SFTP &sftp, const char *name, int access_type, mode_t mode):
     file
     (
      sftp_open
      (
       sftp.get(),
       name,
       access_type,
       mode
      )
     )
    {
     if (file == nullptr)
      sftp.throw_error();
    }

    sftp_file get() const
    {
     return file;
    }

    ~SFTP_File()
    {
     sftp_close(file);
    }
  };
 }
}

#endif
