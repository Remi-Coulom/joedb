#ifndef joedb_Writable_Command_Processor_declared
#define joedb_Writable_Command_Processor_declared

#include "joedb/io/Command_Processor.h"

namespace joedb
{
 class Writable;

 ////////////////////////////////////////////////////////////////////////////
 class Writable_Command_Processor:
 ////////////////////////////////////////////////////////////////////////////
  public Command_Processor
 {
  friend class Readable_Writable_Command_Processor;

  private:
   Writable &writable;
   Writable * const blob_writer;
   bool aborted;

   Status process_command
   (
    const std::string &command,
    std::istream &iss,
    std::ostream &out
   ) final;

  public:
   Writable_Command_Processor
   (
    Writable &writable,
    Writable *blob_writer
   ):
    writable(writable),
    blob_writer(blob_writer),
    aborted(false)
   {}

   bool was_aborted() const {return aborted;}

   ~Writable_Command_Processor();
 };
}

#endif
