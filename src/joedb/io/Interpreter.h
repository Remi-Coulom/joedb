#ifndef joedb_Interpreter_declared
#define joedb_Interpreter_declared

#include "joedb/io/Command_Interpreter.h"
#include "joedb/io/Readable_Command_Processor.h"
#include "joedb/io/Readable_Writable_Command_Processor.h"
#include "joedb/index_types.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Interpreter: public Command_Interpreter
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Readable_Command_Processor readable_command_processor;

  public:
   Readonly_Interpreter
   (
    const Readable &readable,
    Blob_Reader *blob_reader
   ):
    readable_command_processor(readable, blob_reader)
   {
    add_processor(readable_command_processor);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreter: public Command_Interpreter
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Readable_Command_Processor readable_command_processor;
   Readable_Writable_Command_Processor readable_writable_command_processor;

  public:
   Interpreter
   (
    Readable &readable,
    Writable &writable,
    Blob_Reader *blob_reader,
    Writable *blob_writer,
    Record_Id max_record_id
   ):
    readable_command_processor(readable, blob_reader),
    readable_writable_command_processor
    (
     readable,
     writable,
     blob_reader,
     blob_writer,
     max_record_id
    )
   {
    add_processor(readable_command_processor);
    add_processor(readable_writable_command_processor);
   }
 };
}

#endif
