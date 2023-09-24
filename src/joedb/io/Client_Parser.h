#ifndef joedb_Client_Parser_declared
#define joedb_Client_Parser_declared

#include "joedb/io/Connection_Parser.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/Client_Data.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Client_Parser
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Connection_Parser connection_parser;

   std::unique_ptr<Generic_File> client_file;
   std::unique_ptr<Client_Data> client_data;
   std::unique_ptr<Connection> connection;
   std::unique_ptr<Client> client;

  public:
   Client_Parser(bool local, bool readonly);

   Client &parse(int argc, char **argv);

   void print_help(std::ostream &out) const;
 };
}

#endif
