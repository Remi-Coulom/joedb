#include "joedb/compiler/generator/File_Client_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 File_Client_h::File_Client_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "File_Client.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void File_Client_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  namespace_include_guard(out, "File_Client", options.get_name_space());

  out << R"RRR(
#include "Client.h"
#include "joedb/journal/File.h"

)RRR";

  namespace_open(out, options.get_name_space());

  out << R"RRR(
 namespace detail
 {
  ///////////////////////////////////////////////////////////////////////////
  class File_Client_Data
  ///////////////////////////////////////////////////////////////////////////
  {
   protected:
    joedb::File file;
    joedb::Connection connection;

    File_Client_Data(const char *file_name):
     file(file_name, joedb::File::lockable ? joedb::Open_Mode::shared_write : joedb::Open_Mode::write_existing_or_create_new)
    {
    }
  };
 }

 /// Handle concurrent access to a file
 class File_Client: private detail::File_Client_Data, public Client
 {
  public:
   File_Client(const char *file_name):
    detail::File_Client_Data(file_name),
    Client(File_Client_Data::file, File_Client_Data::connection)
   {
   }

   File_Client(const std::string &file_name):
    File_Client(file_name.c_str())
   {
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";
 }
}
