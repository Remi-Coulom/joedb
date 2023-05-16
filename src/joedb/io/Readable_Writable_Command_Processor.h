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
   Writable * const blob_writer;

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
    Writable *blob_writer,
    Record_Id max_record_id
   ):
    Readable_Parser(readable, blob_reader),
    writable(writable),
    blob_writer(blob_writer),
    max_record_id(max_record_id)
   {}

   ~Readable_Writable_Command_Processor();
 };
}

#endif
