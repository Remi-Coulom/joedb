#ifndef joedb_Readable_Command_Processor_declared
#define joedb_Readable_Command_Processor_declared

#include "joedb/ui/Command_Processor.h"
#include "joedb/ui/write_value.h"

namespace joedb
{
 class Readable;

 /// @ingroup ui
 class Readable_Command_Processor: public Command_Processor
 {
  protected:
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
   ) override;

  public:
   Readable_Command_Processor(const Readable &readable): readable(readable)
   {
   }
 };
}

#endif
