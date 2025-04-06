#ifndef joedb_Writable_Command_Processor_declared
#define joedb_Writable_Command_Processor_declared

#include "joedb/ui/Command_Processor.h"

namespace joedb
{
 class Writable;

 /// @ingroup ui
 class Writable_Command_Processor: public Command_Processor
 {
  friend class Readable_Writable_Command_Processor;

  private:
   Writable &writable;
   Writable &blob_writer;

   Status process_command
   (
    const std::string &command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) final;

  public:
   Writable_Command_Processor
   (
    Writable &writable,
    Writable &blob_writer
   ):
    writable(writable),
    blob_writer(blob_writer)
   {}
 };
}

#endif
