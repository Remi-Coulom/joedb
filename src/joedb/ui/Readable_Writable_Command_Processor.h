#ifndef joedb_Readable_Writable_Command_Processor_declared
#define joedb_Readable_Writable_Command_Processor_declared

#include "joedb/ui/Data_Manipulation_Command_Processor.h"
#include "joedb/Type.h"

namespace joedb
{
 /// \ingroup ui
 class Readable_Writable_Command_Processor: public Data_Manipulation_Command_Processor
 {
  private:
   Type parse_type(std::istream &in, std::ostream &out) const;

   Status process_command
   (
    const std::string &command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) override;

  public:
   Readable_Writable_Command_Processor
   (
    const Readable &readable,
    Writable &writable,
    size_t max_record_id
   ):
    Data_Manipulation_Command_Processor(readable, writable, max_record_id)
   {
   }
 };
}

#endif
