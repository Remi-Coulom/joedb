#include "joedb/ui/Writable_Command_Processor.h"
#include "joedb/ui/type_io.h"
#include "joedb/Writable.h"

#include <ctime>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Command_Processor::Status Writable_Command_Processor::process_command
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::string &command,
  std::istream &parameters,
  std::istream &in,
  std::ostream &out
 )
 {
  if (command == "help") ////////////////////////////////////////////////////
  {
   out << R"RRR(Journal
~~~~~~~
 timestamp [<stamp>] (if no value is given, use current time)
 comment "<comment_string>"
 valid_data
 flush
 checkpoint [no|half|full]
 write_blob <data_string>

)RRR";

   return Status::ok;
  }
  else if (command == "comment") ///////////////////////////////////////////
  {
   const std::string comment = read_string(parameters);
   writable.comment(comment);
  }
  else if (command == "timestamp") /////////////////////////////////////////
  {
   int64_t timestamp = 0;
   parameters >> timestamp;
   if (parameters.fail())
    timestamp = std::time(nullptr);
   writable.timestamp(timestamp);
  }
  else if (command == "valid_data") ////////////////////////////////////////
  {
   writable.valid_data();
  }
  else if (command == "flush") /////////////////////////////////////////////
  {
   writable.flush();
  }
  else if (command == "checkpoint") ////////////////////////////////////////
  {
   std::string param;
   parameters >> param;

   if (param == "no")
    writable.checkpoint(Commit_Level::no_commit);
   else if (param == "half")
    writable.checkpoint(Commit_Level::half_commit);
   else if (param == "full")
    writable.checkpoint(Commit_Level::full_commit);
   else if (param.empty())
    writable.default_checkpoint();
   else
    out << "Error: unknown parameter: " << param << '\n';
  }
  else if (command == "write_blob") ////////////////////////////////////////
  {
   const std::string value = read_string(parameters);
   const Blob blob = blob_writer.write_blob_data(value);
   write_blob(out, blob);
   out << '\n';
  }
  else
   return Status::not_found;

  return Status::done;
 }
}
