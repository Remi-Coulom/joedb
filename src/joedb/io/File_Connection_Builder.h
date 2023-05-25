#ifndef joedb_File_Connection_Builder_declared
#define joedb_File_Connection_Builder_declared

#include "joedb/io/Connection_Builder.h"
#include "joedb/concurrency/File_Connection.h"
#include "joedb/journal/File.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 class File_Connection_Data: private File, public File_Connection
 /////////////////////////////////////////////////////////////////////////////
 {
  public:
   File_Connection_Data(const char *server_file_name):
    File(server_file_name, Open_Mode::write_existing_or_create_new),
    File_Connection(*static_cast<File *>(this))
   {
   }
 };

 /////////////////////////////////////////////////////////////////////////////
 class File_Connection_Builder: public Connection_Builder
 /////////////////////////////////////////////////////////////////////////////
 {
  public:
   const char *get_name() const final {return "file";}
   int get_min_parameters() const final {return 1;}
   int get_max_parameters() const final {return 1;}

   const char *get_parameters_description() const final
   {
    return "<file_name>";
   }

   std::unique_ptr<Connection> build(int argc, char **argv) final
   {
    const char * const server_file_name = argv[0];

    return std::unique_ptr<Connection>
    (
     new File_Connection_Data(server_file_name)
    );
   }
 };
}

#endif
