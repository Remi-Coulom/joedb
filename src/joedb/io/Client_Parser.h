#ifndef joedb_Client_Parser_declared
#define joedb_Client_Parser_declared

#include "joedb/io/Connection_Parser.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/Client_Data.h"

#ifdef JOEDB_HAS_SSH
#include "joedb/ssh/Session.h"
#include "joedb/ssh/SFTP.h"
#endif

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Client_Parser
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Connection_Parser connection_parser;

#ifdef JOEDB_HAS_SSH
   std::unique_ptr<ssh::Session> ssh_session;
   std::unique_ptr<ssh::SFTP> sftp;
#endif

   std::unique_ptr<Generic_File> client_file;
   std::unique_ptr<Client_Data> client_data;
   std::unique_ptr<Connection> connection;
   std::unique_ptr<Client> client;

  public:
   Client_Parser(bool local);

   Client &parse(int argc, char **argv);

   void print_help(std::ostream &out) const;
 };
}

#endif
