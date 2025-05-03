#ifndef joedb_ssh_Connector_declared
#define joedb_ssh_Connector_declared

#include "joedb/concurrency/Connector.h"
#include "joedb/ssh/Forward_Channel.h"

namespace joedb::ssh
{
 class Session_And_Channel: public Session, public Forward_Channel
 {
  public:
   Session_And_Channel
   (
    const std::string &user,
    const std::string &host,
    const unsigned port,
    const int verbosity,
    const char * const b64_key,
    const char * const passphrase,
    const char * const remote_path
   ):
    Session(user, host, port, verbosity, b64_key, passphrase),
    Forward_Channel(*this, remote_path)
   {
   }
 };

 /// @ingroup concurrency
 class Connector: public joedb::Connector
 {
  private:
   const std::string user;
   const std::string host;
   const unsigned port;
   const int verbosity;
   const char * const b64_key;
   const char * const passphrase;
   const std::string remote_path;

  public:
   Connector
   (
    std::string user,
    std::string host,
    const unsigned port,
    const int verbosity,
    const char * const b64_key,
    const char * const passphrase,
    std::string remote_path
   ):
    user(std::move(user)),
    host(std::move(host)),
    port(port),
    verbosity(verbosity),
    b64_key(b64_key),
    passphrase(passphrase),
    remote_path(std::move(remote_path))
   {
   }

   std::unique_ptr<Channel> new_channel() const override
   {
    return std::make_unique<Session_And_Channel>
    (
     user,
     host,
     port,
     verbosity,
     b64_key,
     passphrase,
     remote_path.c_str()
    );
   }
 };
}

#endif
