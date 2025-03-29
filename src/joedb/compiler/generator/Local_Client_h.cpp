#include "joedb/compiler/generator/Local_Client_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Local_Client_h::Local_Client_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Local_Client.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Local_Client_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  namespace_include_guard(out, "Local_Client", options.get_name_space());

  out << R"RRR(
#include "Client.h"
#include "joedb/journal/File.h"

)RRR";

  namespace_open(out, options.get_name_space());

  out << R"RRR(
 namespace detail
 {
  ///////////////////////////////////////////////////////////////////////////
  class Local_Client_Data
  ///////////////////////////////////////////////////////////////////////////
  {
   protected:
    joedb::File file;
    joedb::Connection connection;

    Local_Client_Data(const char *file_name):
     file(file_name, joedb::File::lockable ? joedb::Open_Mode::shared_write : joedb::Open_Mode::write_existing_or_create_new)
    {
    }
  };
 }

 /// Handle concurrent access to a file
 class Local_Client: private detail::Local_Client_Data, public Client
 {
  public:
   Local_Client(const char *file_name):
    detail::Local_Client_Data(file_name),
    Client(Local_Client_Data::file, Local_Client_Data::connection)
   {
   }

   Local_Client(const std::string &file_name):
    Local_Client(file_name.c_str())
   {
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";
 }
}
