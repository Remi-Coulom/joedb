#ifndef joedb_ssh_declared
#define joedb_ssh_declared

#include "joedb/error/Exception.h"

#include <libssh/libssh.h>

namespace joedb
{
 namespace ssh
 {
  ///////////////////////////////////////////////////////////////////////////
  inline void check_not_null(void *p)
  ///////////////////////////////////////////////////////////////////////////
  {
   if (!p)
    throw error::Exception("SSH null error");
  }

  ///////////////////////////////////////////////////////////////////////////
  inline void check_ssh_session_result(ssh_session session, int result)
  ///////////////////////////////////////////////////////////////////////////
  {
   if (result != SSH_OK)
    throw error::Exception(ssh_get_error(session));
  }
 }
}

#endif
