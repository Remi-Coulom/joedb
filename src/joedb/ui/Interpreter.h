#ifndef joedb_Interpreter_declared
#define joedb_Interpreter_declared

#include "joedb/ui/Command_Interpreter.h"
#include "joedb/ui/Readable_Command_Processor.h"
#include "joedb/ui/Writable_Command_Processor.h"
#include "joedb/ui/Readable_Writable_Command_Processor.h"

namespace joedb
{
 /// @ingroup ui
 class Readable_Interpreter: public Command_Interpreter
 {
  protected:
   Readable_Command_Processor readable_command_processor;

  public:
   Readable_Interpreter(const Readable &readable):
    readable_command_processor(readable)
   {
    add_processor(readable_command_processor);
   }
 };

 /// @ingroup ui
 class Writable_Interpreter: public Command_Interpreter
 {
  protected:
   Writable_Command_Processor writable_command_processor;

  public:
   Writable_Interpreter(Writable &writable):
    writable_command_processor(writable)
   {
    add_processor(writable_command_processor);
   }
 };

 /// @ingroup ui
 class Interpreter: public Command_Interpreter
 {
  private:
   Writable_Command_Processor writable_command_processor;
   Readable_Writable_Command_Processor readable_writable_command_processor;

  public:
   Interpreter
   (
    const Readable &readable,
    Writable &writable,
    Record_Id max_record_id
   ):
    writable_command_processor(writable),
    readable_writable_command_processor
    (
     readable,
     writable,
     max_record_id
    )
   {
    add_processor(writable_command_processor);
    add_processor(readable_writable_command_processor);
   }
 };
}

#endif
