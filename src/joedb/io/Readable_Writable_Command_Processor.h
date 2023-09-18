#ifndef joedb_Readable_Writable_Command_Processor_declared
#define joedb_Readable_Writable_Command_Processor_declared

#include "joedb/io/Command_Processor.h"
#include "joedb/io/Readable_Command_Processor.h"
#include "joedb/io/Writable_Command_Processor.h"
#include "joedb/index_types.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Readable_Writable_Command_Processor:
 ////////////////////////////////////////////////////////////////////////////
  public Command_Processor
 {
  private:
   Readable_Command_Processor &readable_command_processor;
   Writable_Command_Processor &writable_command_processor;

   const Readable &get_readable()
   {
    return readable_command_processor.readable;
   }

   Writable &get_writable()
   {
    return writable_command_processor.writable;
   }

   Type parse_type(std::istream &in, std::ostream &out) const
   {
    return readable_command_processor.parse_type(in, out);
   }

   Table_Id parse_table(std::istream &in, std::ostream &out) const
   {
    return readable_command_processor.parse_table(in, out);
   }

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

   Size max_record_id;

  public:
   Readable_Writable_Command_Processor
   (
    Readable_Command_Processor &readable_command_processor,
    Writable_Command_Processor &writable_command_processor,
    Size max_record_id
   ):
    readable_command_processor(readable_command_processor),
    writable_command_processor(writable_command_processor),
    max_record_id(max_record_id)
   {}
 };
}

#endif
