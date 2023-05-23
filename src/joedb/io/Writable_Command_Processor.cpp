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
  std::istream &iss,
  std::ostream &out
 )
 {
  if (command == "help") ////////////////////////////////////////////////////
  {
   out << "Journal\n";
   out << "~~~~~~~\n";
   out << " timestamp [<stamp>] (if no value is given, use current time)\n";
   out << " comment \"<comment_string>\"\n";
   out << " valid_data\n";
   out << " checkpoint\n";
   out << " abort\n";
   out << " blob <data_string>\n";
   out << '\n';

   return Status::ok;
  }
  else if (command == "comment") ///////////////////////////////////////////
  {
   const std::string comment = joedb::read_string(iss);
   writable.comment(comment);
  }
  else if (command == "timestamp") /////////////////////////////////////////
  {
   int64_t timestamp = 0;
   iss >> timestamp;
   if (iss.fail())
    timestamp = std::time(nullptr);
   writable.timestamp(timestamp);
  }
  else if (command == "valid_data") ////////////////////////////////////////
  {
   writable.valid_data();
  }
  else if (command == "checkpoint") ////////////////////////////////////////
  {
   writable.checkpoint(Commit_Level::no_commit);
  }
  else if (command == "abort") //////////////////////////////////////////////
  {
   aborted = true;
   return Status::quit;
  }
  else if (command == "blob") ///////////////////////////////////////////////
  {
   const std::string value = joedb::read_string(iss);

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
   writable.checkpoint(Commit_Level::no_commit);
   if (blob_writer && blob_writer != &writable)
    blob_writer->checkpoint(Commit_Level::no_commit);
  }
 }
}
