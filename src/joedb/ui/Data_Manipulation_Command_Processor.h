#ifndef joedb_Data_Manipulation_Command_Processor_declared
#define joedb_Data_Manipulation_Command_Processor_declared

#include "joedb/ui/Readable_Command_Processor.h"

namespace joedb
{
 class Writable;

 /// @ingroup ui
 class Data_Manipulation_Command_Processor: public Readable_Command_Processor
 {
  private:
   void update_value
   (
    std::istream &in,
    Table_Id table_id,
    Record_Id record_id,
    Field_Id field_id
   );

  protected:
   Status process_command
   (
    const std::string &command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) override;

   Writable &writable;
   Record_Id max_record_id;

  public:
   Data_Manipulation_Command_Processor
   (
    const Readable &readable,
    Writable &writable,
    Record_Id max_record_id
   ):
    Readable_Command_Processor(readable),
    writable(writable),
    max_record_id(max_record_id)
   {}
 };
}

#endif
