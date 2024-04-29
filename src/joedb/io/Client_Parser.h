#ifndef joedb_Client_Parser_declared
#define joedb_Client_Parser_declared

#include "joedb/io/File_Parser.h"
#include "joedb/io/Connection_Parser.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/Interpreted_Client_Data.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Client_Parser
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   File_Parser file_parser;
   Connection_Parser connection_parser;

   const Open_Mode default_open_mode;

   std::unique_ptr<Interpreted_Client_Data> client_data;
   std::unique_ptr<Connection> connection;
   std::unique_ptr<Client> client;

  public:
   Client_Parser(bool local, Open_Mode default_open_mode);

   Client &parse(int argc, char **argv);

   void print_help(std::ostream &out) const;
 };
}

#endif
