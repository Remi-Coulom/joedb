#ifndef joedb_ssh_SCP_declared
#define joedb_ssh_SCP_declared

#include "joedb/ssh/ssh.h"

namespace joedb
{
 namespace ssh
 {
  ///////////////////////////////////////////////////////////////////////////
  class SCP
  ///////////////////////////////////////////////////////////////////////////
  {
   private:
    const ssh_scp scp;

   public:
    SCP(ssh_session session, int mode, const char *location):
     scp(ssh_scp_new(session, mode, location))
    {
     check_not_null(scp);
     check_ssh_session_result(session, ssh_scp_init(scp));
    }

    ssh_scp get() const {return scp;}

    ~SCP()
    {
     ssh_scp_close(scp);
     ssh_scp_free(scp);
    }
  };
 }
}

#endif
