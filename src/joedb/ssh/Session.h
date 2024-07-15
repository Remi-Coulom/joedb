#ifndef joedb_ssh_Session_declared
#define joedb_ssh_Session_declared

#include "joedb/ssh/ssh.h"

#include <string>

namespace joedb
{
 namespace ssh
 {
  ///////////////////////////////////////////////////////////////////////////
  class Session_Allocation
  ///////////////////////////////////////////////////////////////////////////
  {
   protected:
    const ssh_session session;

   public:
    Session_Allocation(): session(ssh_new())
    {
     check_not_null(session);
    }

    ssh_session get() const
    {
     return session;
    }

    void check_result(int result) const
    {
     check_ssh_session_result(session, result);
    }

    ~Session_Allocation()
    {
     ssh_free(session);
    }
  };

  ///////////////////////////////////////////////////////////////////////////
  class Session_Connection: public Session_Allocation
  ///////////////////////////////////////////////////////////////////////////
  {
   public:
    Session_Connection
    (
     const char *user,
     const char *host,
     unsigned port,
     int verbosity
    )
    {
     ssh_options_set(session, SSH_OPTIONS_HOST, host);
     ssh_options_set(session, SSH_OPTIONS_USER, user);
     ssh_options_set(session, SSH_OPTIONS_PORT, &port);
     ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

     {
      const int no_delay = 1;
      ssh_options_set(session, SSH_OPTIONS_NODELAY, &no_delay);
     }

     check_result(ssh_connect(session));
    }

    ~Session_Connection()
    {
     ssh_disconnect(session);
    }
  };

  ///////////////////////////////////////////////////////////////////////////
  class Session: public Session_Connection
  ///////////////////////////////////////////////////////////////////////////
  {
   public:
    Session
    (
     const std::string &user,
     const std::string &host,
     unsigned port,
     int verbosity
    ):
     Session_Connection(user.c_str(), host.c_str(), port, verbosity)
    {
     check_result(ssh_userauth_publickey_auto(session, nullptr, nullptr));
    }
  };
 }
}

#endif
