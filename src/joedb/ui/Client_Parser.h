#ifndef joedb_Client_Parser_declared
#define joedb_Client_Parser_declared

#include "joedb/ui/File_Parser.h"
#include "joedb/ui/Connection_Parser.h"
#include "joedb/concurrency/Client.h"

namespace joedb
{
 /// @ingroup ui
 class Client_Parser
 {
  private:
   static constexpr const char *check_string[3] = {"none", "quick", "full"};

   File_Parser file_parser;
   Connection_Parser connection_parser;

   const Open_Mode default_open_mode;
   const bool default_with_database;

   std::unique_ptr<Client> client;

  public:
   Client_Parser(bool local, Open_Mode default_open_mode, bool with_database);

   Client &parse(int argc, char **argv);
   bool has_file() const {return file_parser.get_file() != nullptr;}

   void print_help(std::ostream &out) const;
 };
}

#endif
