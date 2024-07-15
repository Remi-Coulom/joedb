#ifndef joedb_ssh_SFTP_declared
#define joedb_ssh_SFTP_declared

#include "joedb/ssh/Session.h"

#include <libssh/sftp.h>

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
  class SFTP_Allocation
  ///////////////////////////////////////////////////////////////////////////
  {
   private:
    static char const * const error_message[];

   protected:
    const sftp_session sftp;

   public:
    SFTP_Allocation(const Session &session): sftp(sftp_new(session.get()))
    {
     check_not_null(sftp);
    }

    sftp_session get() const
    {
     return sftp;
    }

    void throw_error(const char *message = "sftp error: ") const;

    void check_result(int result) const
    {
     if (result != SSH_OK)
      throw_error();
    }

    ~SFTP_Allocation()
    {
     sftp_free(sftp);
    }
  };

  ///////////////////////////////////////////////////////////////////////////
  class SFTP: public SFTP_Allocation
  ////////////////////////////////////////////////////////////////////////////
  {
   public:
    SFTP(const Session &session): SFTP_Allocation(session)
    {
     check_result(sftp_init(sftp));
    }
  };
 }
}

#endif
