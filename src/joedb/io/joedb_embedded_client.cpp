#include "joedb/io/Connection_Builder.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/client_main.h"
#include "joedb/concurrency/Embedded_Connection.h"
#include "joedb/journal/File.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 class File_Embedded_Connection: private File, public Embedded_Connection
 /////////////////////////////////////////////////////////////////////////////
 {
  public:
   File_Embedded_Connection(const char *server_file_name):
    File(server_file_name, Open_Mode::write_existing_or_create_new),
    Embedded_Connection(*static_cast<File *>(this))
   {
   }
 };

 /////////////////////////////////////////////////////////////////////////////
 class Embedded_Connection_Builder: public Connection_Builder
 /////////////////////////////////////////////////////////////////////////////
 {
  public:
   int get_min_parameters() const final {return 1;}
   int get_max_parameters() const final {return 1;}

   const char *get_parameters_description() const final
   {
    return "<server_file_name>";
   }

   std::unique_ptr<Connection> build(int argc, char **argv) final
   {
    const char * const server_file_name = argv[0];

    return std::unique_ptr<Connection>
    (
     new File_Embedded_Connection(server_file_name)
    );
   }
 };

 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  return client_main(argc, argv, Embedded_Connection_Builder());
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
