#ifndef joedb_ssh_Session_declared
#define joedb_ssh_Session_declared

#include "joedb/ssh/ssh.h"

namespace joedb
{
 namespace ssh
 {
  ///////////////////////////////////////////////////////////////////////////
  class Session
  ///////////////////////////////////////////////////////////////////////////
  {
   private:
    const std::string user;
    const std::string host;
    const ssh_session session;

   public:
    Session
    (
     std::string user,
     std::string host,
     int port,
     int verbosity
    ):
     user(user),
     host(host),
     session(ssh_new())
    {
     check_not_null(session);

     ssh_options_set(session, SSH_OPTIONS_HOST, host.c_str());
     ssh_options_set(session, SSH_OPTIONS_USER, user.c_str());
     ssh_options_set(session, SSH_OPTIONS_PORT, &port);
     ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

     check_result(ssh_connect(session));
     check_result(ssh_userauth_publickey_auto(session, nullptr, nullptr));
    }

    ssh_session get() const
    {
     return session;
    }

    const std::string &get_user() {return user;}
    const std::string &get_host() {return host;}

    void check_result(int result) const
    {
     check_ssh_session_result(session, result);
    }

    ~Session()
    {
     if (session)
      ssh_free(session);
    }
  };
 }
}

#endif
