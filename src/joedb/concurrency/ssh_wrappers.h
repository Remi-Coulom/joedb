#ifndef ssh_wrappers_declared
#define ssh_wrappers_declared

#include <libssh/libssh.h>
#include <libssh/sftp.h>

#include "joedb/Exception.h"

namespace joedb {namespace ssh
{
 ////////////////////////////////////////////////////////////////////////////
 inline void check_not_null(void *p)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!p)
   throw joedb::Exception("SSH null error");
 }

 ////////////////////////////////////////////////////////////////////////////
 class Session
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   const ssh_session session;

  public:
   Session
   (
    const std::string &user,
    const std::string &host,
    int port,
    int verbosity
   ):
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

   void check_result(int result) const
   {
    if (result != SSH_OK)
     throw joedb::Exception(ssh_get_error(session));
   }

   ~Session()
   {
    if (session)
     ssh_free(session);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
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

 ////////////////////////////////////////////////////////////////////////////
 class SFTP_Attributes
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   const sftp_attributes attributes;

  public:
   SFTP_Attributes(sftp_attributes attributes): attributes(attributes)
   {
    check_not_null(attributes);
   }

   sftp_attributes get() const
   {
    return attributes;
   }

   ~SFTP_Attributes()
   {
    if (attributes)
     sftp_attributes_free(attributes);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class SCP
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   const ssh_scp scp;

  public:
   SCP(Session &session, int mode, const char *location):
    scp(ssh_scp_new(session.get(), mode, location))
   {
    check_not_null(scp);
    session.check_result(ssh_scp_init(scp));
   }

   ssh_scp get() const {return scp;}

   ~SCP()
   {
    ssh_scp_close(scp);
    ssh_scp_free(scp);
   }
 };
}}

#endif
