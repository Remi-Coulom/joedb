#ifndef joedb_ssh_declared
#define joedb_ssh_declared

#include "joedb/Exception.h"

#include <libssh/libssh.h>
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

namespace joedb
{
 namespace ssh
 {
  ///////////////////////////////////////////////////////////////////////////
  inline void check_not_null(void *p)
  ///////////////////////////////////////////////////////////////////////////
  {
   if (!p)
    throw joedb::Exception("SSH null error");
  }

  ///////////////////////////////////////////////////////////////////////////
  inline void check_ssh_session_result(ssh_session session, int result)
  ///////////////////////////////////////////////////////////////////////////
  {
   if (result != SSH_OK)
    throw joedb::Exception(ssh_get_error(session));
  }
 }
}

#endif
