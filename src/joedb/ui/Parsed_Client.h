#ifndef joedb_Parsed_Client_declared
#define joedb_Parsed_Client_declared

#include "joedb/ui/File_Parser.h"
#include "joedb/ui/Connection_Parser.h"
#include "joedb/ui/Arguments.h"
#include "joedb/concurrency/Client.h"

namespace joedb
{
//#define PERSISTENCE_TEST
 /// @ingroup ui
 class Parsed_Client
 {
  public:
   enum class DB_Type
   {
    none,
    interpreted,
    dump,
    dump_tail,
    sql,
    sql_tail,
#ifdef PERSISTENCE_TEST
    joedb
#endif
   };

  private:
   const Open_Mode default_open_mode;
   const DB_Type default_db_type;

   File_Parser file_parser;
   Connection_Parser connection_parser;

   std::unique_ptr<Client> client;

  public:
   Parsed_Client
   (
    Logger &logger,
    Open_Mode default_open_mode,
    DB_Type default_db_type,
    Arguments &arguments
   );

   Client *get() {return client.get();}
   bool has_file() const {return file_parser.get_file() != nullptr;}

   void print_help(std::ostream &out) const
   {
    file_parser.print_help(out);
    connection_parser.print_help(out);
   }
 };
}

#endif
