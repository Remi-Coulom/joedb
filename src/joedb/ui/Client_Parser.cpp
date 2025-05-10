#include "joedb/ui/Client_Parser.h"
#include "joedb/ui/Connection_Parser.h"
#include "joedb/ui/SQL_Dump_Writable.h"
#include "joedb/ui/Interpreter_Dump_Writable.h"
#include "joedb/concurrency/Readonly_Database_Client.h"
#include "joedb/concurrency/Writable_Database_Client.h"
#include "joedb/concurrency/Writable_Journal_Client.h"

#ifdef PERSISTENCE_TEST
#include "joedb/journal/File.h"
#endif

#include <iostream>
#include <cstring>

namespace joedb
{
 template<typename Writable>
 class Readonly_Writable_Client: public Readonly_Client
 {
  private:
   Writable writable;

  public:
   Readonly_Writable_Client
   (
    Buffered_File &file,
    Connection &connection,
    Content_Check content_check
   ):
    Readonly_Client(file, connection, content_check),
    writable(std::cout)
   {
    read_journal();
   }

  void read_journal() override
  {
   Readonly_Journal::play_until_checkpoint(writable);
  }
 };

#ifdef PERSISTENCE_TEST
 class Joedb_Client: public Readonly_Client
 {
  private:
   File data_file;
   Writable_Journal writable;

  public:
   Joedb_Client
   (
    Buffered_File &file,
    Connection &connection,
    Content_Check content_check
   ):
    Readonly_Client(file, connection, content_check),
    data_file("joedb.joedb", Open_Mode::write_existing_or_create_new),
    writable(data_file)
   {
    read_journal();
   }

   void read_journal() override
   {
    this->data_journal.play_until_checkpoint(this->writable);
   }
 };
#endif

 ////////////////////////////////////////////////////////////////////////////
 Client_Parser::Client_Parser
 ////////////////////////////////////////////////////////////////////////////
 (
  Open_Mode default_open_mode,
  DB_Type default_db_type,
  Arguments &arguments
 ):
  file_parser
  (
   default_open_mode,
   default_open_mode == Open_Mode::read_existing,
   true,
   true
  ),
  default_open_mode(default_open_mode),
  default_db_type(default_db_type)
 {
  const bool hard_checkpoint = arguments.has_option("hard_checkpoint");

  static std::vector<const char *> check_string
  {
   "none",
   "quick",
   "full"
  };

  const Content_Check content_check = Content_Check
  (
   arguments.get_enum_option
   (
    "check",
    check_string,
    int(Content_Check::quick)
   )
  );
  std::cerr << "content_check = " << check_string[int(content_check)] << '\n';

  static std::vector<const char *> db_string
  {
   "none",
   "interpreted",
   "dump",
   "sql",
#ifdef PERSISTENCE_TEST
   "joedb"
#endif
  };

  const DB_Type db_type = DB_Type
  (
   arguments.get_enum_option
   (
    "db",
    db_string,
    int(default_db_type)
   )
  );
  std::cerr << "db_type = " << db_string[int(db_type)] << '\n';

  file_parser.parse(std::cerr, arguments);

  Buffered_File *client_file = file_parser.get_file();

  Connection &connection = connection_parser.build(arguments, client_file);

  if (!client_file)
   client_file = dynamic_cast<Buffered_File *>(&connection);

  if (!client_file)
   throw Exception("server file must be used with a network or ssh connection");

  std::cerr << "Creating client... ";

  if (db_type == DB_Type::none)
  {
   if (client_file->is_readonly())
   {
    client.reset
    (
     new Readonly_Client(*client_file, connection, content_check)
    );
   }
   else
   {
    client.reset
    (
     new Writable_Journal_Client(*client_file, connection, content_check)
    );
   }
  }
  else if (db_type == DB_Type::interpreted)
  {
   if (client_file->is_readonly())
   {
    client.reset
    (
     new Readonly_Database_Client(*client_file, connection, content_check)
    );
   }
   else
   {
    client.reset
    (
     new Writable_Database_Client(*client_file, connection, content_check)
    );
   }
  }
  else if (db_type == DB_Type::sql)
  {
   client.reset(new Readonly_Writable_Client<joedb::SQL_Dump_Writable>
   (
    *client_file,
    connection,
    content_check
   ));
  }
  else if (db_type == DB_Type::dump)
  {
   client.reset(new Readonly_Writable_Client<joedb::Interpreter_Dump_Writable>
   (
    *client_file,
    connection,
    content_check
   ));
  }
#ifdef PERSISTENCE_TEST
  else if (db_type == DB_Type::joedb)
  {
   client.reset(new Joedb_Client
   (
    *client_file,
    connection,
    content_check
   ));
  }
#endif
  else
   throw Exception("unsupported db type");

  {
   auto *writable_client = dynamic_cast<Writable_Client*>(client.get());
   if (writable_client)
    writable_client->set_hard_checkpoint(hard_checkpoint);
  }

  std::cerr << "OK\n";
 }
}
