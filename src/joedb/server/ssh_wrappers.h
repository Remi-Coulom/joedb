#ifndef ssh_wrappers_declared
#define ssh_wrappers_declared

#include <libssh/libssh.h>
#include <libssh/sftp.h>

#include "joedb/Exception.h"

namespace ssh
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
 class Channel
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   const ssh_channel channel;

  public:
   Channel(const Session& session): channel(ssh_channel_new(session.get()))
   { 
    check_not_null(channel);
    if (ssh_channel_open_session(channel) != SSH_OK)
     throw joedb::Exception("Could not open ssh channel");
   }

  void request_exec(const char *command)
  {
   if (ssh_channel_request_exec(channel, command) != SSH_OK)
    throw joedb::Exception("Could not execute command");
  }

  int get_exit_status()
  {
   return ssh_channel_get_exit_status(channel);
  }

  ~Channel()
  {
   if (channel)
   {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
   }
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
}

#endif
