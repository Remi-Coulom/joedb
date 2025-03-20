#ifndef joedb_Readable_Command_Processor_declared
#define joedb_Readable_Command_Processor_declared

#include "joedb/io/Command_Processor.h"
#include "joedb/io/write_value.h"

namespace joedb
{
 class Readable;

 ////////////////////////////////////////////////////////////////////////////
 class Readable_Command_Processor: public Command_Processor
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Readable_Writable_Command_Processor;

  private:
   const Readable &readable;

   Table_Id parse_table(std::istream &in, std::ostream &out) const;

   void write_value
   (
    std::ostream &out,
    Table_Id table_id,
    Record_Id record_id,
    Field_Id field_id
   )
   {
    joedb::write_value(out, readable, table_id, record_id, field_id);
   }

   Status process_command
   (
    const std::string &command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) final;

  public:
   Readable_Command_Processor(const Readable &readable): readable(readable)
   {
   }
 };
}

#endif
