#include "joedb/io/Blob_Reader_Command_Processor.h"
#include "joedb/io/type_io.h"
#include "joedb/journal/File.h"
#include "joedb/Blob.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Command_Processor::Status Blob_Reader_Command_Processor::process_command
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::string &command,
  std::istream &parameters,
  std::istream &in,
  std::ostream &out
 )
 {
  if (command == "read_blob") ///////////////////////////
  {
   const Blob blob = read_blob(parameters);

   if (!blob.is_null())
   {
    const std::string s = blob_reader.read_blob_data(blob);
    const std::string file_name = read_string(parameters);

    if (file_name.empty())
    {
     write_string(out, s);
     out << '\n';
    }
    else
    {
     File file(file_name, joedb::Open_Mode::create_new);
     file.write_data(s.data(), s.size());
     file.flush();
    }
   }
  }
  else if (command == "help") ///////////////////////////////////////////////
  {
   out << R"RRR(Blob Reader
~~~~~~~~~~~
 read_blob <blob> [<output_file_name>]\n";

)RRR";

   return Status::ok;
  }
  else
   return Status::not_found;

  return Status::done;
 }
}
