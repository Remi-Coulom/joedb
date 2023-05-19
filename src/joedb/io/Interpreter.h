#ifndef joedb_Interpreter_declared
#define joedb_Interpreter_declared

#include "joedb/io/Command_Interpreter.h"
#include "joedb/io/Readable_Command_Processor.h"
#include "joedb/io/Writable_Command_Processor.h"
#include "joedb/io/Readable_Writable_Command_Processor.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Readable_Interpreter:
 ////////////////////////////////////////////////////////////////////////////
  private Readable_Command_Processor,
  public Command_Interpreter
 {
  public:
   Readable_Interpreter
   (
    const Readable &readable,
    Blob_Reader *blob_reader
   ):
    Readable_Command_Processor(readable, blob_reader),
    Command_Interpreter{*static_cast<Readable_Command_Processor *>(this)}
   {
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Writable_Interpreter:
 ////////////////////////////////////////////////////////////////////////////
  private Writable_Command_Processor,
  public Command_Interpreter
 {
  public:
   Writable_Interpreter(Writable &writable):
    Writable_Command_Processor(writable, nullptr),
    Command_Interpreter{*static_cast<Writable_Command_Processor *>(this)}
   {
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreter:
 ////////////////////////////////////////////////////////////////////////////
  private Readable_Command_Processor,
  private Writable_Command_Processor,
  private Readable_Writable_Command_Processor,
  public Command_Interpreter
 {
  public:
   Interpreter
   (
    const Readable &readable,
    Writable &writable,
    Blob_Reader *blob_reader,
    Writable *blob_writer,
    Record_Id max_record_id
   ):
    Readable_Command_Processor(readable, blob_reader),
    Writable_Command_Processor(writable, blob_writer),
    Readable_Writable_Command_Processor(*this, *this, max_record_id),
    Command_Interpreter
    {
     *static_cast<Readable_Command_Processor *>(this),
     *static_cast<Writable_Command_Processor *>(this),
     *static_cast<Readable_Writable_Command_Processor *>(this)
    }
   {
   }
 };
}

#endif
