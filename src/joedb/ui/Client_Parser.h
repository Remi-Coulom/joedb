#ifndef joedb_Client_Parser_declared
#define joedb_Client_Parser_declared

#include "joedb/ui/File_Parser.h"
#include "joedb/ui/Connection_Parser.h"
#include "joedb/concurrency/Client.h"

namespace joedb
{
//#define PERSISTENCE_TEST
 /// @ingroup ui
 class Client_Parser // TODO: rename into "parsed client"
 {
  public:
   enum class DB_Type
   {
    none,
    interpreted,
    dump,
    sql,
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
   Client_Parser
   (
    Open_Mode default_open_mode,
    DB_Type default_db_type,
    Arguments &arguments
   );

   Client *get() {return client.get();}
   bool has_file() const {return file_parser.get() != nullptr;}

   void print_help(std::ostream &out) const;
 };
}

#endif
