#ifndef joedb_Readable_Writable_Command_Processor_declared
#define joedb_Readable_Writable_Command_Processor_declared

#include "joedb/io/Command_Processor.h"
#include "joedb/io/Readable_Parser.h"

namespace joedb
{
 class Writable;

 ////////////////////////////////////////////////////////////////////////////
 class Readable_Writable_Command_Processor:
 ////////////////////////////////////////////////////////////////////////////
  public Command_Processor,
  public Readable_Parser
 {
  private:
   Writable &writable;

   void update_value
   (
    std::istream &in,
    Table_Id table_id,
    Record_Id record_id,
    Field_Id field_id
   );

   Status process_command
   (
    const std::string &command,
    std::istream &iss,
    std::ostream &out
   ) final;

   Record_Id max_record_id;

  public:
   Readable_Writable_Command_Processor
   (
    Readable &readable,
    Writable &writable,
    Blob_Reader *blob_reader,
    Record_Id max_record_id
   ):
    Readable_Parser(readable),
    writable(writable),
    max_record_id(max_record_id)
   {}
 };
}

#endif
