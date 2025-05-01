#include "joedb/ui/Client_Parser.h"
#include "joedb/ui/Connection_Parser.h"
#include "joedb/ui/SQL_Dump_Writable.h"
#include "joedb/ui/Interpreter_Dump_Writable.h"
#include "joedb/concurrency/Readonly_Database_Client.h"
#include "joedb/concurrency/Writable_Database_Client.h"
#include "joedb/concurrency/Readonly_Journal_Client.h"
#include "joedb/concurrency/Writable_Journal_Client.h"
#include "joedb/concurrency/Client.h"

#include <iostream>
#include <cstring>

namespace joedb
{
 template<typename Writable>
 class Writable_Readonly_Client_Data
 {
  protected:
   Readonly_Journal data_journal;
   Writable writable;

   Writable_Readonly_Client_Data(Buffered_File &file):
    data_journal(file),
    writable(std::cout)
   {
   }
 };

 template<typename Writable>
 class Writable_Readonly_Client:
  private Writable_Readonly_Client_Data<Writable>,
  public Readonly_Client
 {
  public:
   Writable_Readonly_Client
   (
    Buffered_File &file,
    Connection &connection,
    Content_Check content_check
   ):
    Writable_Readonly_Client_Data<Writable>(file),
    Readonly_Client(this->data_journal, connection, content_check)
   {
    read_journal();
   }

  void read_journal() override
  {
   this->data_journal.play_until_checkpoint(this->writable);
  }
 };

 ////////////////////////////////////////////////////////////////////////////
 Client_Parser::Client_Parser
 ////////////////////////////////////////////////////////////////////////////
 (
  Open_Mode default_open_mode,
  DB_Type default_db_type
 ):
  file_parser(default_open_mode, default_open_mode == Open_Mode::read_existing, true, true),
  connection_parser(),
  default_open_mode(default_open_mode),
  default_db_type(default_db_type)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Client_Parser::print_help(std::ostream &out) const
 ////////////////////////////////////////////////////////////////////////////
 {
  out << " [--check ";

  for (size_t i = 0; i < std::size(check_string); i++)
  {
   if (i > 0)
    out << '|';
   out << check_string[i];
  }

  out << "] [--db ";

  for (size_t i = 0; i < std::size(db_string); i++)
  {
   if (i > 0)
    out << '|';
   out << db_string[i];
  }


  out << "] <file> <connection>\n\n";

  file_parser.print_help(out);
  connection_parser.print_help(out);
 }

 ////////////////////////////////////////////////////////////////////////////
 Client &Client_Parser::parse(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  int arg_index = 0;

  Content_Check content_check = Content_Check::quick;
  if (arg_index + 1 < argc && std::strcmp(argv[arg_index], "--check") == 0)
  {
   arg_index++;
   for (size_t i = 0; i < std::size(check_string); i++)
    if (std::strcmp(argv[arg_index], check_string[i]) == 0)
     content_check = Content_Check(i);
   arg_index++;
  }
  std::cerr << "content_check = " << check_string[int(content_check)] << '\n';

  DB_Type db_type = default_db_type;
  if (arg_index + 1 < argc && std::strcmp(argv[arg_index], "--db") == 0)
  {
   arg_index++;
   for (size_t i = 0; i < std::size(db_string); i++)
    if (std::strcmp(argv[arg_index], db_string[i]) == 0)
     db_type = DB_Type(i);
   arg_index++;
  }
  std::cerr << "db_type = " << db_string[int(db_type)] << '\n';

  file_parser.parse
  (
   std::cerr,
   argc,
   argv,
   arg_index
  );

  Buffered_File *client_file = file_parser.get_file();

  Connection &connection = connection_parser.build
  (
   argc - arg_index,
   argv + arg_index,
   client_file
  );

  if (!client_file)
   client_file = dynamic_cast<Buffered_File *>(&connection);

  if (!client_file)
   throw Exception("server file must be used with a network or ssh connection");

  std::cerr << "Creating client data... ";
  std::cerr << "OK\n";

  if (db_type == DB_Type::none)
  {
   if (client_file->is_readonly())
   {
    client.reset
    (
     new Readonly_Journal_Client(*client_file, connection, content_check)
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
   client.reset(new Writable_Readonly_Client<joedb::SQL_Dump_Writable>
   (
    *client_file,
    connection,
    content_check
   ));
  }
  else if (db_type == DB_Type::dump)
  {
   client.reset(new Writable_Readonly_Client<joedb::Interpreter_Dump_Writable>
   (
    *client_file,
    connection,
    content_check
   ));
  }
  else
   throw Exception("unsupported db type");

  return *client;
 }
}
