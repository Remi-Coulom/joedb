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
   std::unique_ptr<Embedded_Connection> connection;

  public:
   int get_min_parameters() const override {return 1;}
   int get_max_parameters() const override {return 2;}

   const char *get_parameters_description() const override
   {
    return "<server.joedb> [<client.joedb>]";
   }

   void build(int argc, const char * const *argv) override
   {
    const char * const server_file_name = argv[0];
    const char * const client_file_name = argc > 1 ? argv[1] : nullptr;

    open_local_file(client_file_name);
    server_file.reset
    (
     new File(server_file_name, Open_Mode::write_existing_or_create_new)
    );
    connection.reset(new Embedded_Connection(*server_file));
   }

   Connection &get_connection() override {return *connection;}
   Generic_File &get_file() override {return *local_file;}
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
