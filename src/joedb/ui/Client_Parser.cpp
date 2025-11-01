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
 template<typename Writable_Multiplexer>
 class Readonly_Writable_Client: public Readonly_Client
 {
  private:
   Writable_Multiplexer writable_multiplexer;

  public:
   Readonly_Writable_Client
   (
    Abstract_File &file,
    Connection &connection,
    bool tail,
    Content_Check content_check,
    Recovery recovery
   ):
    Readonly_Client(file, connection, content_check, recovery),
    writable_multiplexer(std::cout)
   {
    if (tail)
     writable_multiplexer.set_start_index(1);

    read_journal();

    if (tail)
     writable_multiplexer.set_start_index(0);
   }

  void read_journal() override
  {
   Readonly_Journal::play_until_checkpoint(writable_multiplexer);
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
    Abstract_File &file,
    Connection &connection,
    Content_Check content_check,
    Recovery recovery
   ):
    Readonly_Client(file, connection, content_check, recovery),
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
  Logger &logger,
  Open_Mode default_open_mode,
  DB_Type default_db_type,
  Arguments &arguments
 ):
  default_open_mode(default_open_mode),
  default_db_type(default_db_type),
  file_parser
  (
   default_open_mode,
   default_open_mode == Open_Mode::read_existing,
   true,
   true
  )
 {
  const bool hard_checkpoint = arguments.has_flag("hard_checkpoint");

  static std::vector<const char *> check_string
  {
   "none",
   "fast",
   "full"
  };

  const Content_Check content_check = Content_Check
  (
   arguments.get_enum_option
   (
    "check",
    check_string,
    int(Content_Check::fast)
   )
  );

  static std::vector<const char *> recovery_string
  {
   "none",
   "ignore_header",
   "overwrite"
  };

  const Recovery recovery = Recovery
  (
   arguments.get_enum_option
   (
    "recovery",
    recovery_string,
    int(Recovery::none)
   )
  );

  static std::vector<const char *> db_string
  {
   "none",
   "interpreted",
   "dump",
   "dump_tail",
   "sql",
   "sql_tail",
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

  arguments.add_parameter("<file>");
  arguments.add_parameter("<connection>");

  if (arguments.get_remaining_count() == 0)
   return;

  logger.write("hard_checkpoint = " + std::to_string(hard_checkpoint));
  logger.write("content_check = " + std::string(check_string[int(content_check)]));
  logger.write("recovery = " + std::string(recovery_string[int(recovery)]));
  logger.write("db_type = " + std::string(db_string[int(db_type)]));

  Abstract_File *client_file = file_parser.parse(logger, arguments);
  Connection *connection = connection_parser.build(logger, arguments, client_file);

  if (!client_file)
   client_file = dynamic_cast<Abstract_File *>(connection);

  if (!client_file)
   throw Exception("could not create file");

  if (!connection)
   throw Exception("could not create connection");

  logger.write("creating client");

  if (db_type == DB_Type::none)
  {
   if (client_file->is_readonly())
   {
    client.reset
    (
     new Readonly_Client(*client_file, *connection, content_check, recovery)
    );
   }
   else
   {
    client.reset
    (
     new Writable_Journal_Client
     (
      *client_file,
      *connection,
      content_check,
      recovery
     )
    );
   }
  }
  else if (db_type == DB_Type::interpreted)
  {
   if (client_file->is_readonly())
   {
    client.reset
    (
     new Readonly_Database_Client
     (
      *client_file,
      *connection,
      content_check,
      recovery
     )
    );
   }
   else
   {
    client.reset
    (
     new Writable_Database_Client
     (
      *client_file,
      *connection,
      content_check,
      Record_Id::null,
      recovery
     )
    );
   }
  }
  else if (db_type == DB_Type::sql || db_type == DB_Type::sql_tail)
  {
   client.reset(new Readonly_Writable_Client<joedb::SQL_Dump_Writable>
   (
    *client_file,
    *connection,
    db_type == DB_Type::sql_tail,
    content_check,
    recovery
   ));
  }
  else if (db_type == DB_Type::dump || db_type == DB_Type::dump_tail)
  {
   client.reset(new Readonly_Writable_Client<joedb::Interpreter_Dump_Writable>
   (
    *client_file,
    *connection,
    db_type == DB_Type::dump_tail,
    content_check,
    recovery
   ));
  }
#ifdef PERSISTENCE_TEST
  else if (db_type == DB_Type::joedb)
  {
   client.reset(new Joedb_Client
   (
    *client_file,
    connection,
    content_check,
    recovery
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
 }
}
