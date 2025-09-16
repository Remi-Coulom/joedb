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
 void File_Client_h::write(std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  namespace_include_guard_open(out, "File_Client", options.get_name_space());

  out << R"RRR(
#include "Client.h"
#include "joedb/journal/File.h"

)RRR";

  namespace_open(out, options.get_name_space());

  out << R"RRR(
 /// Shortcut to directly build a @ref Client from a file name
 class File_Client: private joedb::File, public Client
 {
  public:
   File_Client(const char *file_name):
    joedb::File
    (
     file_name,
     joedb::File::lockable
     ? joedb::Open_Mode::shared_write
     : joedb::Open_Mode::write_existing_or_create_new
    ),
    Client(*this, joedb::Connection::dummy)
   {
   }

   File_Client(const std::string &file_name):
    File_Client(file_name.c_str())
   {
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  namespace_include_guard_close(out);
 }
}
