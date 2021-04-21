#ifndef joedb_ssh_SFTP_declared
#define joedb_ssh_SFTP_declared

#include "joedb/ssh/Session.h"

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

    void check_result(int result) const
    {
     if (result != SSH_OK)
      throw joedb::Exception
      (
       "SFTP error code: " + std::to_string(sftp_get_error(sftp))
      );
    }

    ~SFTP()
    {
     if (sftp)
      sftp_free(sftp);
    }
  };
 }
}

#endif
