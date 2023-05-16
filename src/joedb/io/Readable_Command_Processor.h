#ifndef joedb_Readable_Command_Processor_declared
#define joedb_Readable_Command_Processor_declared

#include "joedb/io/Command_Processor.h"
#include "joedb/io/Readable_Parser.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Readable_Command_Processor:
 ////////////////////////////////////////////////////////////////////////////
  public Command_Processor,
  public Readable_Parser
 {
  private:
   Blob_Reader *blob_reader;

   Status process_command
   (
    const std::string &command,
    std::istream &iss,
    std::ostream &out
   ) final;

  public:
   Readable_Command_Processor
   (
    const Readable &readable,
    Blob_Reader *blob_reader
   ):
    Readable_Parser(readable),
    blob_reader(blob_reader)
   {
   }
 };
}

#endif
