#ifndef joedb_ssh_SFTP_declared
#define joedb_ssh_SFTP_declared

#include "joedb/ssh/Session.h"

#include <libssh/sftp.h>
#include <string>

//
// Windows does not define those
//
#include <fcntl.h>
#ifndef S_IRUSR
#define S_IRUSR 0000400
#endif

#ifndef S_IWUSR
#define S_IWUSR 0000200
#endif

#ifndef S_IRWXU
#define S_IRWXU 0000700
#endif

namespace joedb
{
 namespace ssh
 {
  ///////////////////////////////////////////////////////////////////////////
  class SFTP
  ////////////////////////////////////////////////////////////////////////////
  {
   private:
    const sftp_session sftp;

   public:
    SFTP(const Session &session): sftp(sftp_new(session.get()))
    {
     check_not_null(sftp);
     check_result(sftp_init(sftp));
    }

    sftp_session get() const
    {
     return sftp;
    }

    void throw_error() const
    {
     throw joedb::Exception
     (
      "SFTP error code: " + std::to_string(sftp_get_error(sftp))
     );
    }

    void check_result(int result) const
    {
     if (result != SSH_OK)
      throw_error();
    }

    ~SFTP()
    {
     if (sftp)
      sftp_free(sftp);
    }
  };

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
