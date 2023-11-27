#ifndef joedb_Readable_Command_Processor_declared
#define joedb_Readable_Command_Processor_declared

#include "joedb/io/Command_Processor.h"
#include "joedb/Type.h"

namespace joedb
{
 class Readable;
 class Blob_Reader;

 ////////////////////////////////////////////////////////////////////////////
 class Readable_Command_Processor: public Command_Processor
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Readable_Writable_Command_Processor;

  private:
   const Readable &readable;
   Blob_Reader *blob_reader;

   Type parse_type(std::istream &in, std::ostream &out) const;
   Table_Id parse_table(std::istream &in, std::ostream &out) const;

   Status process_command
   (
    const std::string &command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) final;

  public:
   Readable_Command_Processor
   (
    const Readable &readable,
    Blob_Reader *blob_reader
   ):
    readable(readable),
    blob_reader(blob_reader)
   {
   }
 };
}

#endif
