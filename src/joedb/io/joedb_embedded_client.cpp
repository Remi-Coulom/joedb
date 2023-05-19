#include "joedb/io/Connection_Builder.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/concurrency/Embedded_Connection.h"
#include "joedb/journal/File.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 class Embedded_Connection_Builder: public Connection_Builder
 /////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::unique_ptr<File> server_file;
   std::unique_ptr<Writable_Journal> server_journal;

  public:
   int get_min_parameters() const final {return 1;}
   int get_max_parameters() const final {return 1;}
   const char *get_parameters_description() const final
   {
    return "<server_file_name>";
   }

   std::unique_ptr<Connection> build
   (
    Writable_Journal &client_journal,
    int argc,
    const char * const *argv
   ) final
   {
    const char * const server_file_name = argv[0];

    server_file.reset
    (
     new File(server_file_name, Open_Mode::write_existing_or_create_new)
    );

    server_journal.reset(new Writable_Journal(*server_file));

    return std::unique_ptr<Connection>
    (
     new Embedded_Connection(client_journal, *server_journal)
    );
   }
 };

 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  return Embedded_Connection_Builder().main(argc, argv);
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
