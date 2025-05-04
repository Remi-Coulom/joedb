#ifndef joedb_Client_Parser_declared
#define joedb_Client_Parser_declared

#include "joedb/ui/File_Parser.h"
#include "joedb/ui/Connection_Parser.h"
#include "joedb/concurrency/Client.h"

namespace joedb
{
//#define PERSISTENCE_TEST
 /// @ingroup ui
 class Client_Parser
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
   static constexpr const char *check_string[] =
   {
    "none",
    "quick",
    "full"
   };

   static constexpr const char *db_string[] =
   {
    "none",
    "interpreted",
    "dump",
    "sql",
#ifdef PERSISTENCE_TEST
    "joedb"
#endif
   };

   File_Parser file_parser;
   Connection_Parser connection_parser;

   const Open_Mode default_open_mode;
   const DB_Type default_db_type;
   static constexpr Content_Check default_content_check = Content_Check::quick;

   std::unique_ptr<Client> client;

  public:
   Client_Parser
   (
    Open_Mode default_open_mode,
    DB_Type default_db_type
   );

   Client &parse(int argc, const char * const * argv);
   bool has_file() const {return file_parser.get_file() != nullptr;}

   void print_help(std::ostream &out) const;
 };
}

#endif
