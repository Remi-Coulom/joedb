#ifndef joedb_Interpreter_declared
#define joedb_Interpreter_declared

#include "joedb/ui/Command_Interpreter.h"
#include "joedb/ui/Readable_Command_Processor.h"
#include "joedb/ui/Writable_Command_Processor.h"
#include "joedb/ui/Readable_Writable_Command_Processor.h"
#include "joedb/ui/Blob_Reader_Command_Processor.h"

#include <memory>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Readable_Interpreter: public Command_Interpreter
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   Readable_Command_Processor readable_command_processor;
   std::unique_ptr<Blob_Reader_Command_Processor> blob_reader_command_processor;

  public:
   Readable_Interpreter
   (
    const Readable &readable,
    Blob_Reader *blob_reader
   ):
    readable_command_processor(readable)
   {
    add_processor(readable_command_processor);
    if (blob_reader)
    {
     blob_reader_command_processor = std::make_unique<Blob_Reader_Command_Processor>(*blob_reader);
     add_processor(*blob_reader_command_processor);
    }
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Writable_Interpreter: public Command_Interpreter
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   Writable_Command_Processor writable_command_processor;

  public:
   Writable_Interpreter(Writable &writable, Blob_Writer &blob_writer):
    writable_command_processor(writable, blob_writer)
   {
    add_processor(writable_command_processor);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreter: public Readable_Interpreter
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Writable_Command_Processor writable_command_processor;
   Readable_Writable_Command_Processor readable_writable_command_processor;

  public:
   Interpreter
   (
    const Readable &readable,
    Writable &writable,
    Blob_Reader *blob_reader,
    Blob_Writer &blob_writer,
    size_t max_record_id
   ):
    Readable_Interpreter(readable, blob_reader),
    writable_command_processor(writable, blob_writer),
    readable_writable_command_processor
    (
     readable_command_processor,
     writable_command_processor,
     max_record_id
    )
   {
    add_processor(writable_command_processor);
    add_processor(readable_writable_command_processor);
   }
 };
}

#endif
