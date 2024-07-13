#include "joedb/io/Writable_Command_Processor.h"
#include "joedb/io/type_io.h"
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
 checkpoint
 abort
 blob <data_string>

)RRR";

   return Status::ok;
  }
  else if (command == "comment") ///////////////////////////////////////////
  {
   const std::string comment = joedb::read_string(parameters);
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
   writable.default_checkpoint();
  }
  else if (command == "abort") //////////////////////////////////////////////
  {
   aborted = true;
   return Status::quit;
  }
  else if (command == "blob") ///////////////////////////////////////////////
  {
   const std::string value = joedb::read_string(parameters);

   const Blob blob = blob_writer ?
    blob_writer->write_blob_data(value) :
    writable.write_blob_data(value);

   joedb::write_blob(out, blob);
   out << '\n';
  }
  else
   return Status::not_found;

  return Status::done;
 }

 ////////////////////////////////////////////////////////////////////////////
 Writable_Command_Processor::~Writable_Command_Processor()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!aborted)
  {
   writable.default_checkpoint();
   if (blob_writer && blob_writer != &writable)
    blob_writer->default_checkpoint();
  }
 }
}
