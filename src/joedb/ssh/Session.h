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

    Session_Allocation(const Session_Allocation &) = delete;
    Session_Allocation &operator=(const Session_Allocation &) = delete;

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
  class Imported_Key
  ///////////////////////////////////////////////////////////////////////////
  {
   private:
    ssh_key key;

   public:
    Imported_Key(const char *b64_key, const char *passphrase): key(nullptr)
    {
     ssh_pki_import_privkey_base64
     (
      b64_key,
      passphrase,
      nullptr,
      nullptr,
      &key
     );

     if (key == nullptr)
      throw joedb::Exception("Could not import private key");
    }

    Imported_Key(const Imported_Key &) = delete;
    Imported_Key &operator=(const Imported_Key &) = delete;

    ssh_key get() const
    {
     return key;
    }

    ~Imported_Key()
    {
     ssh_key_free(key);
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
     const unsigned port,
     const int verbosity,
     const char * const b64_key = nullptr,
     const char * const passphrase = nullptr
    ):
     Session_Connection(user.c_str(), host.c_str(), port, verbosity)
    {
     if (b64_key)
     {
      const Imported_Key key(b64_key, passphrase);
      check_result(ssh_userauth_publickey(session, user.c_str(), key.get()));
     }
     else
     {
      check_result(ssh_userauth_publickey_auto(session, nullptr, passphrase));
     }
    }
  };
 }
}

#endif
